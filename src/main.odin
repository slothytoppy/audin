package termaudio
import "./id3"
import "core:fmt"
import "core:os"
import "core:strconv"
import "core:strings"
import rl "vendor:raylib"

main :: proc() {
	// id3.todo("rename project to audin (credit from wiru)")
	id3.todo("rename project to audin")
	rl.SetTraceLogLevel(rl.TraceLogLevel.ERROR)
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
	rl.PlayMusicStream(q.music)
	assert(rl.IsMusicStreamPlaying(q.music) == true)
	for (!rl.WindowShouldClose()) {
		buf: [8]byte
		rl.UpdateMusicStream(q.music)
		rl.BeginDrawing()
		/*
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
    */
		write_volume(q, rl.Vector2{0, 0})
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
