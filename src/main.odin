package termaudio
import "./id3"
import "core:fmt"
import "core:os"
import "core:path/filepath"
import "core:strconv"
import "core:strings"
import "song_queue"
import rl "vendor:raylib"


main :: proc() {
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
	q.paths = song_queue.read_dir("../stuff/")
	fmt.println(q.paths.files[:])
	q.count = cast(u64)len(q.paths.files)
	song_queue._read_dir("hello", nil)
	fmt.println(q.count)
	q = play_song(q)
	defer unload_song(q)
	q.volume = 0.5
	q.playing = true
	q.muted = false
	looping := false
	for (!rl.WindowShouldClose()) {
		buf: [8]byte
		rl.UpdateMusicStream(q.music)
		rl.BeginDrawing()
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
		} else if (rl.IsKeyPressed(.G)) {
			rl.SeekMusicStream(q.music, rl.GetMusicTimeLength(q.music))
		} else if (rl.IsKeyPressed(.M)) {
			q.muted = !q.muted
			volume := q.muted ? 0 : q.volume
			rl.SetMasterVolume(volume)
		} /*else if (rl.IsKeyPressed(.L)) {
			looping = !looping
		}
		if (looping == true && rl.GetMusicTimePlayed(q.music) == rl.GetMusicTimeLength(q.music)) {
		}
    */
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
