package termaudio
// import "vendor:miniaudio"

Audio :: struct {
	//	device:                 ma.device,
	//decoder:                ma.decoder,
	is_device_ready:        bool,
	at_end:                 bool,
	playing:                bool,
	muted:                  bool,
	volume:                 f32,
	cursor:                 i64,
	length:                 u64,
	played_time:            u64,
	length_in_seconds:      u64,
	played_time_in_seconds: u64,
	// queue:                  queue,
}

audio := Audio{}

/*

import "base:runtime"
import "core:fmt"
import "core:os"
import "core:strings"
import ma "vendor:miniaudio"

queue :: struct {
	songs:  [dynamic]string,
	cursor: u64,
}

data_callback :: proc "c" (device: ^ma.device, output: rawptr, input: rawptr, frames_count: u32) {
	context = runtime.default_context()
	decoder := cast(^ma.decoder)device.pUserData
	if (at_song_end() || audio.playing == false) {
		audio.at_end = true
		return
	}
	frames_read: u64
	result := ma.decoder_read_pcm_frames(decoder, output, cast(u64)frames_count, &frames_read)
	if (result !=
		   ma.result.SUCCESS) {fmt.print("count:", frames_count, "read:", frames_read, "\nerror:", result, "\n")
		logger(cast(string)ma.result_description(result))
		fmt.print(len(ma.result))
	}
	assert(result == ma.result.SUCCESS)
	result = ma.decoder_get_cursor_in_pcm_frames(&audio.decoder, &audio.cursor)
	assert(result == ma.result.SUCCESS)
}

init_audio :: proc() {
	config: ma.device_config = ma.device_config_init(ma.device_type.playback)
	config.playback.format = ma.format.f32
	config.playback.channels = 0
	config.sampleRate = 0
	config.dataCallback = data_callback
	config.pUserData = nil
	result: ma.result = ma.device_init(nil, &config, &audio.device)
	assert(result != ma.result.ERROR)
	result = ma.device_start(&audio.device)
	assert(result != ma.result.ERROR)
	audio.is_device_ready = true
	set_volume(0.5)
}

@(private)
assert_is_audio_ready :: proc() {
	assert(audio.is_device_ready == true)
}

play_song :: proc(song_name: string) -> rawptr {
	assert_is_audio_ready()
	assert(os.exists(song_name))
	song := strings.unsafe_string_to_cstring(song_name)
	config := ma.decoder_config_init_default()
	result: ma.result = ma.decoder_init_file(song, &config, &audio.decoder)
	if (result != ma.result.SUCCESS) {
		logger(cast(string)ma.result_description(result))
	}
	assert(result == ma.result.SUCCESS)
	audio.device.pUserData = &audio.decoder
	audio.length = get_song_length()
	audio.playing = true
	audio.at_end = false
	return nil
}

play_next :: proc() {
	if (audio.queue.cursor < 0 || audio.queue.cursor > cast(u64)len(audio.queue.songs)) {
		return
	}
	audio.queue.cursor += 1
	play_song(audio.queue.songs[audio.queue.cursor])
}

unload_song :: proc(song_name: cstring) {
	assert_is_audio_ready()
	ma.decoder_uninit(&audio.decoder)
	audio.playing = false
}

unload_current_song :: proc() {
	ma.decoder_uninit(&audio.decoder)
	audio.playing = false
}

toggle_pause :: proc() {
	audio.playing = !audio.playing
}

toggle_mute :: proc() {
	assert_is_audio_ready()
	audio.muted = !audio.muted
}

@(private)
set_device_volume :: proc(volume: f32) {
	assert_is_audio_ready()
	if (volume <= 1.0 && volume >= 0.0) {
		result := ma.device_set_master_volume(&audio.device, volume)
		assert(result == ma.result.SUCCESS)
	}
}

set_volume :: proc(volume: f32) -> ma.result {
	assert_is_audio_ready()
	result: ma.result
	if (volume <= 1.0 && volume >= 0.0) {
		result = ma.device_set_master_volume(&audio.device, volume)
		assert(result == ma.result.SUCCESS)
		audio.volume = volume
	}
	return result
}

get_volume :: proc() -> f32 {
	assert_is_audio_ready()
	volume: f32
	result := ma.device_get_master_volume(&audio.device, &volume)
	assert(result == ma.result.SUCCESS)
	return volume
}

get_song_length :: proc() -> u64 {
	assert_is_audio_ready()
	length: u64
	result: ma.result = ma.decoder_get_length_in_pcm_frames(&audio.decoder, &length)
	assert(result == ma.result.SUCCESS)
	return length
}

get_song_length_in_seconds :: proc() -> u64 {
	assert_is_audio_ready()
	return get_song_length() / cast(u64)audio.decoder.outputSampleRate
}

get_song_length_in_minutes :: proc() -> u64 {
	assert_is_audio_ready()
	return get_song_length_in_seconds() / 60
}

get_song_played_time :: proc() -> u64 {
	assert_is_audio_ready()
	return audio.cursor / cast(u64)audio.decoder.outputSampleRate
}

get_song_played_time_in_seconds :: proc() -> u64 {
	assert_is_audio_ready()
	return get_song_played_time() / 60
}

get_song_played_time_in_minutes :: proc() -> u64 {
	assert_is_audio_ready()
	return get_song_played_time_in_seconds() / 60
}

at_song_end :: proc() -> bool {
	if (audio.at_end == true || audio.cursor > 0 && audio.cursor == audio.length) {
		return true
	}
	return false
}

/*
main :: proc() {
	errmsg := "figure out linking miniaudio and ncurses with odin"
	queue: queue
	fmt.print("count ", len(queue.item))
	/*
	device := ma.device{}
	config := ma.device_config{}
	config = ma.device_config_init(ma.device_type.playback)
	ma.device_init(nil, &config, &device)
  */
	engine := ma.engine{}
	config: ma.engine_config = ma.engine_config_init()
	ma.engine_init(&config, &engine)
	ma.engine_play_sound(&engine, queue.item[0], nil)

	buf: [256]byte
	num_bytes, err := os.read(os.stdin, buf[:])
	str := string(buf[:num_bytes - 1])
	if (len(str) > 0) {
		return
	}

}
*/
*/
