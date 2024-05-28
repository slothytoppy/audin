package termaudio
import "./id3"
import "core:fmt"
import "core:os"
import "core:strconv"
import "core:strings"
import rl "vendor:raylib"

SongQueue :: struct {
	files:          rl.FilePathList,
	cursor:         i64,
	muted, playing: bool,
	volume:         f32,
	music:          rl.Music,
}

main :: proc() {
	logger("log", "hello world", 2, "\n")
	// id3.todo("rename project to audin (credit from wiru)")
	q := SongQueue{}
	rl.InitAudioDevice()
	defer rl.CloseAudioDevice()
	window_config: rl.ConfigFlags = {rl.ConfigFlag.WINDOW_RESIZABLE}
	rl.InitWindow(1080, 720, "hello window")
	defer rl.CloseWindow()
	font: rl.Font = rl.LoadFont("./fonts/Alegreya-Regular.ttf")
	defer rl.UnloadFont(font)
	rl.SetWindowState(window_config)
	rl.SetTargetFPS(60)
	rl.SetTraceLogLevel(rl.TraceLogLevel.ERROR)
	dir_path: cstring = "../stuff"
	q.files = rl.LoadDirectoryFiles(dir_path)
	id3_info := id3.parse_song(strings.clone_from_cstring(q.files.paths[q.cursor]))
	defer rl.UnloadDirectoryFiles(q.files)
	q.music = rl.LoadMusicStream(q.files.paths[q.cursor])
	defer rl.UnloadMusicStream(q.music)
	assert(rl.IsMusicReady(q.music))
	q.volume = 0.5
	q.playing = true
	rl.SetMasterVolume(q.volume)
	rl.SetMusicVolume(q.music, q.volume)
	rl.PlayMusicStream(q.music)
	assert(rl.IsMusicStreamPlaying(q.music) == true)
	for (!rl.WindowShouldClose()) {
		buf: [8]byte
		rl.UpdateMusicStream(q.music)
		rl.BeginDrawing()
		text := strconv.ftoa(buf[:], cast(f64)q.volume, 'f', 2, 64)
		text, _ = strings.remove(text, "+", 1)
		msg := strings.clone_to_cstring(text)
		rl.DrawTextEx(font, msg, rl.Vector2{0, 0}, cast(f32)font.baseSize, 2, rl.BLUE)
		rl.DrawTextEx(
			font,
			msg,
			rl.Vector2{0, cast(f32)font.baseSize},
			cast(f32)font.baseSize,
			2,
			rl.GREEN,
		)
		rl.ClearBackground(rl.BLACK)
		rl.EndDrawing()
		key := rl.GetKeyPressed()
		if (key == rl.KeyboardKey.ESCAPE || key == rl.KeyboardKey.Q) {
			return
		} else if (rl.IsKeyPressed(.A)) || rl.IsKeyPressedRepeat(.A) {
			q = play_prev(q)
		} else if (rl.IsKeyPressed(.D) || rl.IsKeyPressedRepeat(.D)) {
			q = play_next(q)
		} else if (rl.IsKeyPressed(.C) || rl.IsKeyPressedRepeat(.C)) {
			q.volume -= 0.1
			q.volume = rl.Clamp(q.volume, 0.0, 1.0)
			rl.SetMasterVolume(q.volume)
		} else if (rl.IsKeyPressed(.V) || rl.IsKeyPressedRepeat(.V)) {
			q.volume += 0.1
			q.volume = rl.Clamp(q.volume, 0.0, 1.0)
			rl.SetMasterVolume(q.volume)
		} else if (rl.IsKeyPressed(.P) || rl.IsKeyPressed(.SPACE)) {
			q.playing = !q.playing
			if (q.playing == true) {
				rl.ResumeMusicStream(q.music)
			} else {
				rl.PauseMusicStream(q.music)
			}
		}
	}
}

at_music_end :: proc(q: SongQueue) -> bool {
	q := q
	music_length := rl.GetMusicTimeLength(q.music)
	music_remaining_time := rl.GetMusicTimePlayed(q.music)
	if (music_remaining_time == music_length) {
		return true
	}
	return false
}

/*
   toggle_pause :: proc(q: SongQueue) {
	q := q
	q.playing = !q.playing
}


play_song :: proc(q: SongQueue, song: string) {
	q := q
	q.music = rl.LoadMusicStream(strings.clone_to_cstring(song))
	rl.SetMusicVolume(q.music, q.volume)
	rl.SetMasterVolume(q.volume)
	rl.PlayMusicStream(q.music)
}

*/
@(require_results)
play_prev :: proc(q: SongQueue) -> SongQueue {
	q := q
	q.cursor -= 1
	if (q.cursor < 0) {
		q.cursor = cast(i64)q.files.count - 1
	}

	q.music = rl.LoadMusicStream(q.files.paths[q.cursor])
	rl.PlayMusicStream(q.music)
	assert(rl.IsMusicReady(q.music))
	assert(rl.IsMusicStreamPlaying(q.music))
	fmt.printf("playing %s\n", q.files.paths[q.cursor])
	return q
}

@(require_results)
play_next :: proc(q: SongQueue) -> SongQueue {
	q := q
	q.cursor += 1
	if (q.cursor + 1 >= cast(i64)q.files.count) {
		q.cursor = 0
	}
	q.music = rl.LoadMusicStream(q.files.paths[q.cursor])
	rl.PlayMusicStream(q.music)
	assert(rl.IsMusicReady(q.music))
	assert(rl.IsMusicStreamPlaying(q.music))
	fmt.printf("playing %s\n", q.files.paths[q.cursor])
	return q
}
