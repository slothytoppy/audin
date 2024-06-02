package termaudio
import "core:fmt"
import "core:strings"
import "song_queue"
import rl "vendor:raylib/"

main :: proc() {
	rl.SetTraceLogLevel(rl.TraceLogLevel.ERROR)
	/*
	fmt.println(
		"raylib doesnt allow me to properly get the end of the song so i must rewrite my audio lib",
		"check to make sure that the organization of structures like SongQueue makes sense",
		"write a gui lib for creating buttons, sliders, and progress bars but im unhappy with how raylibs rgui does this",
		sep = "\n\n",
	)
  */
	//unimplemented("rewrite my audio library")
	q := SongQueue {
		volume = 0.5,
		muted  = false,
	}
	rl.InitAudioDevice()
	defer rl.CloseAudioDevice()
	rl.InitWindow(1080, 720, "hello window")
	defer rl.CloseWindow()
	font: rl.Font = rl.LoadFont("./fonts/Alegreya-Regular.ttf")
	defer rl.UnloadFont(font)
	rl.SetTargetFPS(60)
	q.paths = song_queue.read_dir("./stuff/")
	q = play_song(q.files[q.cursor], q)
	defer unload_song(q)
	last_cursor := q.cursor
	font_size: rl.Vector2
	song := q.paths.base_path[q.cursor]
	current_song := strings.clone_to_cstring(song)
	font_size = rl.MeasureTextEx(font, current_song, f32(font.baseSize), f32(font.glyphPadding))
	for (!rl.WindowShouldClose()) {
		rl.BeginDrawing()
		rl.ClearBackground(rl.BLACK)
		rl.UpdateMusicStream(q.music)
		center_x := rl.GetScreenWidth() / 2
		center_y := rl.GetScreenHeight() / 2
		if (at_music_end(q)) {
			q = play_next(q)
		}
		key := rl.GetKeyPressed()
		q = handle_keypress(key, q)
		if (last_cursor != q.cursor) {
			song = q.paths.base_path[q.cursor]
			current_song = strings.clone_to_cstring(song)
			font_size = rl.MeasureTextEx(
				font,
				current_song,
				f32(font.baseSize),
				f32(font.glyphPadding),
			)
			last_cursor = q.cursor
		}
		rl.DrawLineEx(
			rl.Vector2{0, cast(f32)center_y},
			rl.Vector2{font_size.x, cast(f32)center_y},
			1,
			rl.BLUE,
		)
		rl.DrawLineEx(
			rl.Vector2{0, cast(f32)center_y + font_size.y},
			rl.Vector2{font_size.x, cast(f32)center_y + font_size.y},
			1,
			rl.BLUE,
		)
		rl.DrawLineEx(
			rl.Vector2{0, cast(f32)center_y},
			rl.Vector2{1, cast(f32)center_y + font_size.y},
			1,
			rl.BLUE,
		)
		rl.DrawLineEx(
			rl.Vector2{font_size.x, cast(f32)center_y + font_size.y},
			rl.Vector2{font_size.x, cast(f32)center_y},
			1,
			rl.BLUE,
		)
		rl.DrawTextEx(
			font,
			current_song,
			rl.Vector2{0, cast(f32)center_y},
			f32(font.baseSize),
			f32(font.glyphPadding),
			rl.RED,
		)
		rl.EndDrawing()
	}
}

at_music_end :: proc(q: SongQueue) -> bool {
	time_played := rl.GetMusicTimePlayed(q.music) / rl.GetMusicTimeLength(q.music)
	logger("log", rl.GetMusicTimePlayed(q.music), rl.GetMusicTimeLength(q.music))
	{
		assert(rl.GetMusicTimePlayed(q.music) < rl.GetMusicTimeLength(q.music))
	}
	if (time_played > 1.0) {
		fmt.println(true)
		return true
	}
	return false
}
