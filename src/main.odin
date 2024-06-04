package termaudio
import "core:fmt"
import "core:mem"
import "core:strconv"
import "core:strings"
import "song_queue"
import rl "vendor:raylib/"

main :: proc() {
	when ODIN_DEBUG {
		track: mem.Tracking_Allocator
		mem.tracking_allocator_init(&track, context.allocator)
		context.allocator = mem.tracking_allocator(&track)
		{
			defer {
				if len(track.allocation_map) > 0 {
					fmt.eprintf("=== %v allocations not freed: ===\n", len(track.allocation_map))
					for _, entry in track.allocation_map {
						fmt.eprintf("- %v bytes @ %v\n", entry.size, entry.location)
					}
				}
				if len(track.bad_free_array) > 0 {
					fmt.eprintf("=== %v incorrect frees: ===\n", len(track.bad_free_array))
					for entry in track.bad_free_array {
						fmt.eprintf("- %p @ %v\n", entry.memory, entry.location)
					}
				}
				mem.tracking_allocator_destroy(&track)
			}
		}
	}
	/*
	fmt.println(
		"raylib doesnt allow me to properly get the end of the song so i must rewrite my audio lib",
		"check to make sure that the organization of structures like SongQueue makes sense",
		"write a gui lib for creating buttons, sliders, and progress bars but im unhappy with how raylibs rgui does this",
		sep = "\n\n",
	)
  */
	rl.SetTraceLogLevel(rl.TraceLogLevel.ERROR)
	q := SongQueue {
		volume = 0.5,
		muted  = false,
	}
	rl.InitAudioDevice()
	defer rl.CloseAudioDevice()
	flags := rl.ConfigFlag.WINDOW_RESIZABLE
	flag_bits := rl.ConfigFlags{flags}
	rl.InitWindow(1080, 720, "hello window")
	rl.SetWindowState(flag_bits)
	defer rl.CloseWindow()
	font: rl.Font = rl.LoadFont("./fonts/Alegreya-Regular.ttf")
	defer rl.UnloadFont(font)
	rl.SetTargetFPS(60)
	song_queue.read_dir("./stuff/", &q.paths)
	q = play_song(q.paths.fullpath[q.cursor], q)
	defer unload_song(q)
	last_cursor := q.cursor
	font_size: rl.Vector2
	song := q.paths.base_path[q.cursor]
	current_song := strings.clone_to_cstring(song)
	font_size = rl.MeasureTextEx(font, current_song, f32(font.baseSize), 0)
	center_x := rl.GetScreenWidth() / 2
	center_y := rl.GetScreenHeight() / 2
	logger("log", "x:", center_x, "y:", center_y)
	for (!rl.WindowShouldClose()) {
		rl.BeginDrawing()
		rl.ClearBackground(rl.BLACK)
		rl.UpdateMusicStream(q.music)
		if (rl.IsWindowResized()) {
			center_x = rl.GetScreenWidth() / 2
			center_y = rl.GetScreenHeight() / 2
			logger("log", "x:", center_x, "y:", center_y)
		}
		key := rl.GetKeyPressed()
		q = handle_keypress(key, q)
		if (last_cursor != q.cursor) {
			song = q.paths.base_path[q.cursor]
			current_song = strings.clone_to_cstring(song)
			font_size = rl.MeasureTextEx(font, current_song, f32(font.baseSize), 0)
			last_cursor = q.cursor
			logger(
				"log",
				"song:",
				song,
				"font size:",
				font_size.x,
				font_size.y,
				"last cursor",
				last_cursor,
			)
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
			0,
			rl.RED,
		)
		buf: [8]byte
		rl.DrawText(
			strings.clone_to_cstring(strconv.itoa(buf[:], len(q.paths.base_path))),
			center_x,
			center_y,
			cast(i32)font_size.y,
			rl.GREEN,
		)
		rl.DrawText(
			strings.clone_to_cstring(strconv.itoa(buf[:], cast(int)q.cursor + 1)),
			center_x + cast(i32)font_size.y,
			center_y + cast(i32)font_size.y,
			cast(i32)font_size.y,
			rl.GREEN,
		)
		rl.EndDrawing()
	}
}
