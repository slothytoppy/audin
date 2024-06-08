package termaudio
import "audio"
import "core:fmt"
import "core:strings"
import "gui"
import "log"
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
	rl.SetTraceLogLevel(rl.TraceLogLevel.ERROR)
	q := audio.SongQueue {
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
	font: rl.Font = rl.LoadFont("/home/slothy/gh/audin/fonts/Alegreya-Regular.ttf")
	defer rl.UnloadFont(font)
	rl.SetTargetFPS(60)

	audio.read_dir("/home/slothy/gh/audin/stuff/", &q.paths)
	q.music = audio.play_song(q.paths.fullpath[0])
	defer audio.unload_song(q)
	last_cursor := q.cursor
	font_size: rl.Vector2
	song := audio.base_name(q.paths, q.cursor)
	current_song := strings.clone_to_cstring(song)

	font_size = rl.MeasureTextEx(font, current_song, f32(font.baseSize), 0)
	center_x := rl.GetScreenWidth() / 2
	center_y := rl.GetScreenHeight() / 2
	log.logger("log", "x:", center_x, "y:", center_y)
	gui.set_font(font)

	gui.window_state.fullscreen = false
	fmt.println("max_len", audio.get_queue_max_len(q.paths))
	for (!rl.WindowShouldClose()) {
		rl.BeginDrawing()
		defer rl.EndDrawing()
		rl.ClearBackground(rl.BLACK)
		rl.UpdateMusicStream(q.music)
		if (rl.IsWindowResized()) {
			center_x = rl.GetScreenWidth() / 2
			center_y = rl.GetScreenHeight() / 2
			log.logger("log", "x:", center_x, "y:", center_y)
		}
		key := rl.GetKeyPressed()
		q = gui.handle_keypress(key, q)
		if (last_cursor != q.cursor) {
			song = audio.base_name(q.paths, q.cursor)
			current_song = strings.clone_to_cstring(song)
			font_size = rl.MeasureTextEx(font, current_song, f32(font.baseSize), 0)
			last_cursor = q.cursor
			log.logger("log", "font size:", font_size.x, font_size.y, "last cursor", last_cursor)
		}

		textbox_color: gui.textbox_color = {
			box  = rl.BLUE,
			text = rl.RED,
		}
		gui.textbox(
			song,
			gui.pos{center_x - (cast(i32)font_size.x / 2), center_y},
			audio.get_queue_max_len(q.paths),
			textbox_color,
		)
		gui.draw_float(q.volume, gui.pos{center_x * 2 - cast(i32)font_size.y, center_y}, rl.GREEN)
		gui.draw_int(q.cursor, gui.pos{0, center_y + cast(i32)font_size.y}, rl.GRAY)
	}
}
