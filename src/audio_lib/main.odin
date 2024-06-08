package audio_lib

import "base:intrinsics"
import "base:runtime"
import "core:fmt"
import "core:os"
import "core:strconv"
import "core:strings"
import "core:sys/unix"
import ma "vendor:miniaudio"
import rl "vendor:raylib"

Audio :: struct {
	device:                    ma.device,
	decoder:                   ma.decoder,
	resource_manager:          ma.resource_manager,
	frames_count, frames_read: u64,
	ready:                     bool,
	memory:                    rawptr,
	file:                      string,
	volume:                    f32,
	remaining, played:         f64,
	song_queue:                Audio_Queue,
}

Audio_Queue :: struct {
	data:   [dynamic]string,
	cursor: u64,
}

AUDIO: Audio

append_queue :: proc(queue: ^Audio_Queue, data: string) {
	append(&queue.data, data)
	append(&AUDIO.song_queue.data, data)
}

read_dir :: proc(queue: ^Audio_Queue, dirpath: string, recurse: bool = false) {
	fd, _ := os.open(dirpath)
	fi, _ := os.read_dir(fd, 0)
	for i in 0 ..< len(fi) {
		if (fi[i].is_dir) {
			read_dir(queue, fi[i].fullpath)
		} else {
			append(&queue.data, fi[i].fullpath)
		}
	}
}

/*
init_device :: proc(audio: Audio) -> bool
deinit_device :: proc(audio: Audio) -> bool
init_queue :: proc(queue: queue.Queue) -> bool
deinit_queue :: proc(queue: queue.Queue) -> bool
init_audio :: proc(audio: Audio, filename: string, streamed_audio: bool) -> bool
deinit_audio :: proc(audio: Audio) -> bool
append_song :: proc(queue: queue.Queue) -> bool
set_volume :: proc(audio: Audio, volume: f64) -> bool
*/

callback :: proc "c" (device: ^ma.device, output, input: rawptr, frame_count: u32) {
	context = runtime.default_context()
	decoder: ^ma.decoder = cast(^ma.decoder)device.pUserData
	ma.decoder_read_pcm_frames(decoder, output, cast(u64)frame_count, nil)
	ma.decoder_get_cursor_in_pcm_frames(decoder, &AUDIO.frames_read)
}

init_device :: proc(q: ^Audio) {
	q := q
	q.volume = 0.5
	q.ready = false
	cfg: ma.device_config = ma.device_config_init(ma.device_type.playback)
	cfg.playback.format = ma.format.f32
	cfg.playback.channels = 0
	cfg.sampleRate = 0
	cfg.dataCallback = callback
	assert(ma.device_init(nil, &cfg, &q.device) == ma.result.SUCCESS)
	assert(ma.device_start(&q.device) == ma.result.SUCCESS)
}

deinit_device :: proc(device: ma.device) {
	device := device
	ma.device_uninit(&device)
}

load_file :: proc(q: ^Audio, file: string) {
	cfg: ma.decoder_config = ma.decoder_config_init_default()
	assert(
		ma.decoder_init_file(strings.clone_to_cstring(file), &cfg, &q.decoder) ==
		ma.result.SUCCESS,
	)
	q.device.pUserData = cast(rawptr)&q.decoder
	q.ready = true
	ma.decoder_get_length_in_pcm_frames(&q.decoder, &q.frames_count)
}

reset_decoder :: proc(q: ^Audio) {
	ma.decoder_uninit(&q.decoder)
	q.ready = false
}

audio_at_end :: proc(q: Audio) -> bool {
	if (q.frames_count > 0 && q.frames_count == q.frames_read) {
		return true
	} else {
		return false
	}
}

set_volume :: proc(q: ^Audio, volume: f32) {
	q := q
	assert(q.ready == true)
	q.volume = volume
	ma.device_set_master_volume(&q.device, q.volume)
}


pos :: struct {
	x, y: i32,
}

curr_font: rl.Font = rl.GetFontDefault()

set_font :: proc(file: string) {
	curr_font = rl.LoadFont(strings.clone_to_cstring(file))
}

draw_int :: proc(data: $T, pos: pos, color: rl.Color) where intrinsics.type_is_numeric(T) {
	buf: [8]byte
	text: string = strconv.itoa(buf[:], cast(int)data)
	if (data < 10) {
		text = strings.concatenate({"0", text})
	}
	msg: cstring = strings.clone_to_cstring(text)
	sz := rl.MeasureTextEx(curr_font, msg, cast(f32)curr_font.baseSize, 0)
	height := sz.y
	rl.DrawTextEx(curr_font, msg, rl.Vector2{cast(f32)pos.x, cast(f32)pos.y}, height, 0, color)
	delete(msg)
}

main :: proc() {
	rl.InitWindow(400, 800, "ur mom")
	cfg: rl.ConfigFlags = {rl.ConfigFlag.WINDOW_RESIZABLE, rl.ConfigFlag.WINDOW_ALWAYS_RUN}
	rl.SetConfigFlags(cfg)
	read_dir(&AUDIO.song_queue, "../../stuff")
	last_index :: proc(path, substr: string) -> string {
		ret := strings.last_index(path, substr)
		return path[ret + 1:]
	}
	init_device(&AUDIO)
	load_file(&AUDIO, AUDIO.song_queue.data[0])
	fmt.println(len(AUDIO.song_queue.data))
	set_font("../../fonts/Alegreya-Regular.ttf")
	font_height := rl.MeasureTextEx(curr_font, " ", 32, 0)
	fmt.println(font_height.y)
	sample_rate := AUDIO.device.sampleRate
	for {
		rl.BeginDrawing()
		rl.ClearBackground(rl.BLACK)
		draw_int(AUDIO.frames_read / cast(u64)sample_rate, pos{0, 0}, rl.GREEN)
		rl.DrawTextEx(
			curr_font,
			"/",
			rl.Vector2{0, font_height.y},
			cast(f32)curr_font.baseSize,
			0,
			rl.BLUE,
		)
		draw_int(
			AUDIO.frames_count / cast(u64)sample_rate,
			pos{0, cast(i32)font_height.y * 2},
			rl.GREEN,
		)
		key := rl.GetKeyPressed()
		if (key == .Q) {
			rl.CloseWindow()
			return
		}
		if (key == .C || rl.IsKeyPressedRepeat(.C)) {
			AUDIO.volume = rl.Clamp(AUDIO.volume, 0, 1)
			set_volume(&AUDIO, AUDIO.volume - 0.1)
		}
		if (key == .V || rl.IsKeyPressedRepeat(.V)) {
			set_volume(&AUDIO, AUDIO.volume + 0.1)
		}
		if (audio_at_end(AUDIO) == true) {
			AUDIO.song_queue.cursor += 1
			load_file(&AUDIO, AUDIO.song_queue.data[AUDIO.song_queue.cursor])
			fmt.println("play next song!")
		}
		rl.EndDrawing()
	}
	for i in AUDIO.song_queue.data {
		fmt.println(last_index(i, "/"))
	}
}
