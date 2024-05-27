package termaudio
import "./id3"
import "core:fmt"
import "core:os"
import "core:strconv"
import "core:strings"
import "core:thread"
import rl "vendor:raylib"

SongQueue :: struct {
	files:          rl.FilePathList,
	cursor:         i64,
	muted, playing: bool,
	volume:         f32,
	music:          rl.Music,
}

main :: proc() {
	q := SongQueue{}
	rl.InitAudioDevice()
	defer rl.CloseAudioDevice()
	rl.SetTraceLogLevel(rl.TraceLogLevel.ERROR)
	assert(rl.IsAudioDeviceReady() != true)
	q.files = rl.LoadDirectoryFiles(strings.clone_to_cstring("../stuff"))
	id3.parse_song(strings.clone_from_cstring(q.files.paths[0]))
	defer rl.UnloadDirectoryFiles(q.files)
	using rl.KeyboardKey
	q.music = rl.LoadMusicStream(q.files.paths[q.cursor])
	defer rl.UnloadMusicStream(q.music)
	assert(rl.IsMusicReady(q.music))
	q.volume = 0.5
	q.playing = true
	rl.SetMasterVolume(q.volume)
	rl.SetMusicVolume(q.music, q.volume)
	rl.PlayMusicStream(q.music)
	assert(rl.IsMusicStreamPlaying(q.music) == true)
	rl.InitWindow(720, 480, "hello window")
	rl.SetTargetFPS(60)
	/*init_audio()
	q := audio
	q.queue = read_dir("../stuff")
	assert(len(q.queue.songs) > 0)
	play_song(q.queue.songs[0])
  */
	for (!rl.WindowShouldClose()) {
		rl.UpdateMusicStream(q.music)
		rl.BeginDrawing()
		rl.ClearBackground(rl.RED)
		rl.EndDrawing()
		key := get_key_pressed()
		if (key == rl.KeyboardKey.ESCAPE || key == rl.KeyboardKey.Q) {
			rl.CloseWindow()
		}
		if (rl.IsKeyPressed(.C) || rl.IsKeyPressedRepeat(.C)) {
			if (q.volume - 0.1 > 0.0) {
				q.volume -= 0.1
				rl.SetMusicVolume(q.music, q.volume)
				rl.SetMasterVolume(q.volume)
				fmt.println(q.volume)
			}
		} else if (rl.IsKeyPressed(.V) || rl.IsKeyPressedRepeat(.V)) {
			if (audio.volume + 0.1 < 1.1) {
				q.volume += 0.1
				rl.SetMusicVolume(q.music, q.volume)
				rl.SetMasterVolume(q.volume)
				fmt.println(q.volume)
			}
		} else if (rl.IsKeyPressed(.A)) {
			q = play_prev(q)
			id3.parse_song(strings.clone_from_cstring(q.files.paths[q.cursor]))
		} else if (rl.IsKeyPressed(.D) || rl.IsKeyPressedRepeat(.D)) {
			if (rl.IsMusicReady(q.music)) {
				rl.UnloadMusicStream(q.music)
			}
			q = play_next(q)
			id3.parse_song(strings.clone_from_cstring(q.files.paths[q.cursor]))
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
