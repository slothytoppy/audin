package termaudio
import "./id3"
import "core:c"
import "core:fmt"
import "core:os"
import "core:path/filepath"
import "core:strconv"
import "core:strings"
import "song_queue"
import rl "vendor:raylib/"

main :: proc() {
	rl.SetTraceLogLevel(rl.TraceLogLevel.ERROR)
	q := SongQueue {
		volume  = 0.5,
		playing = true,
		muted   = false,
		looping = true,
	}
	rl.InitAudioDevice()
	defer rl.CloseAudioDevice()
	window_config: rl.ConfigFlags = {rl.ConfigFlag.WINDOW_RESIZABLE}
	rl.InitWindow(1080, 720, "hello window")
	defer rl.CloseWindow()
	font: rl.Font = rl.LoadFont("./fonts/Alegreya-Regular.ttf")
	defer rl.UnloadFont(font)
	rl.SetWindowState(window_config)
	rl.SetTargetFPS(60)
	q.paths = song_queue.read_dir("./stuff/")
	q = play_song(q)
	defer unload_song(q)
	for (!rl.WindowShouldClose()) {
		rl.UpdateMusicStream(q.music)
		rl.BeginDrawing()
		rl.ClearBackground(
			rl.GetColor(
				cast(u32)rl.GuiGetStyle(
					cast(c.int)rl.GuiControl.DEFAULT,
					cast(c.int)rl.GuiDefaultProperty.TEXT_SPACING,
				),
			),
		)
		key := rl.GetKeyPressed()
		q = handle_keypress(key, q)
		write_looping(q.looping, rl.Vector2{0, 100})
		write_volume(q, rl.Vector2{0, 0})
		// write_volume(q, rl.Vector2{0, 0})
		rl.DrawTextEx(
			font,
			strings.clone_to_cstring(q.paths.files[q.cursor]),
			rl.Vector2{0, 50},
			cast(f32)font.baseSize,
			0,
			rl.GREEN,
		)
		if (at_music_end(q)) {
			if (q.looping == true) {
				q = play_song(q)
			} else {
				q = play_next(q)
			}
		}
		/*
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
		} else if (rl.IsKeyPressed(.G)) {
			rl.SeekMusicStream(q.music, rl.GetMusicTimeLength(q.music))
		} else if (rl.IsKeyPressed(.M)) {
			q.muted = !q.muted
			volume := q.muted ? 0 : q.volume
			rl.SetMasterVolume(volume)
		} else if (rl.IsKeyPressed(.L)) {
			q.looping = !q.looping
		}
    */

		rl.EndDrawing()
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
