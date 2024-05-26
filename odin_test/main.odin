package termaudio
import "core:fmt"
import "core:os"
import "core:strconv"
import "core:strings"
import "core:thread"
import rl "vendor:raylib"

SongQueue :: struct {
	files:          rl.FilePathList,
	cursor:         u64,
	muted, playing: bool,
	volume:         f32,
	music:          rl.Music,
}

main :: proc() {
	/*
	q := SongQueue{}
	rl.InitAudioDevice()
	defer rl.CloseAudioDevice()
	assert(rl.IsAudioDeviceReady())
	q.files = rl.LoadDirectoryFiles(strings.clone_to_cstring("../stuff/"))
	defer rl.UnloadDirectoryFiles(q.files)
	using rl.KeyboardKey
	q.music = rl.LoadMusicStream(q.files.paths[q.cursor])
	defer rl.UnloadMusicStream(q.music)
	assert(rl.IsMusicReady(q.music))
	rl.SetMasterVolume(q.volume)
	rl.SetMusicVolume(q.music, q.volume)
	rl.PlayMusicStream(q.music)
	assert(rl.IsMusicStreamPlaying(q.music) == true)
	q.volume = 0.5
	rl.InitWindow(720, 480, "hello window")
	rl.SetTargetFPS(60)
  */
	q := audio
	init_audio()
	play_song(q.queue.songs[q.cursor])
	for (!rl.WindowShouldClose()) {
		rl.BeginDrawing()
		rl.ClearBackground(rl.RED)
		key := get_key_pressed()
		if (key == rl.KeyboardKey.ESCAPE || key == rl.KeyboardKey.Q) {
			rl.CloseWindow()
		}
		if (rl.IsKeyPressed(.C) || rl.IsKeyPressedRepeat(.C)) {
			if (q.volume - 0.1 > 0.0) {
				/*
				q.volume -= 0.1
				rl.SetMusicVolume(q.music, q.volume)
				rl.SetMasterVolume(q.volume)
				fmt.println(q.volume)
        */
			}
		} else if (rl.IsKeyPressed(.V) || rl.IsKeyPressedRepeat(.V)) {
			if (audio.volume + 0.1 < 1.1) {
				q.volume += 0.1
				/*
				rl.SetMusicVolume(q.music, q.volume)
				rl.SetMasterVolume(q.volume)
        */
				fmt.println(q.volume)
			}
		} else if (rl.IsKeyPressed(.D)) {
			/*
			assert(rl.IsMusicReady(q.music))
			q = play_next(q)
      */
		} else if (rl.IsKeyPressed(.P) || rl.IsKeyPressed(.SPACE)) {
			q.playing = !q.playing
			/*
			if (q.playing == true) {
				rl.ResumeMusicStream(q.music)
			} else {
				rl.PauseMusicStream(q.music)
			}
      */
		}
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

play_next :: proc(q: SongQueue) -> SongQueue {
	q := q
	if (q.cursor + 1 > cast(u64)q.files.count) {
		return q
	}
	q.cursor += 1
	q.music = rl.LoadMusicStream(q.files.paths[q.cursor])
	rl.SetMusicVolume(q.music, q.volume)
	rl.SetMasterVolume(q.volume)
	rl.PlayMusicStream(q.music)
	return q
}

*/
