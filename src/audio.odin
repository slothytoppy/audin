package termaudio

import "core:os"
import "core:strings"
import "song_queue"
import rl "vendor:raylib"

SongQueue :: struct {
	paths:          song_queue.file_path_list,
	cursor:         i64,
	muted, playing: bool,
	volume:         f32,
	music:          rl.Music,
	looping:        bool,
}

toggle_playing :: proc(q: SongQueue) {
	q := q
	q.playing = !q.playing
	if (q.playing == true) {
		rl.PauseMusicStream(q.music)
	} else {
		rl.ResumeMusicStream(q.music)
	}
}

toggle_looping :: proc(q: SongQueue) {
	q := q
	q.looping = !q.looping
}

@(require_results)
play_song :: proc(q: SongQueue) -> SongQueue {
	q := q
	assert(os.exists(q.paths.files[q.cursor]), q.paths.files[q.cursor])
	q.music = rl.LoadMusicStream(strings.clone_to_cstring(q.paths.files[q.cursor]))
	rl.PlayMusicStream(q.music)
	logger("log", "playing:", q.paths.files[q.cursor])
	return q
}

@(require_results)
play_prev :: proc(q: SongQueue) -> SongQueue {
	q := q
	q.cursor -= 1
	if (q.cursor < 0) {
		q.cursor = cast(i64)q.paths.count - 1
	}
	q = play_song(q)
	return q
}

@(require_results)
play_next :: proc(q: SongQueue) -> SongQueue {
	q := q
	q.cursor += 1
	if (q.cursor + 1 > cast(i64)q.paths.count) {
		q.cursor = 0
	}
	q = play_song(q)
	return q
}

unload_song :: proc(q: SongQueue) {
	rl.UnloadMusicStream(q.music)
}

Change_Volume_Bits :: enum {
	NEGATIVE,
	POSITIVE,
}

change_volume :: proc(
	volume_flag: Change_Volume_Bits,
	q: SongQueue,
	step: f32 = 0.1,
) -> SongQueue {
	q := q
	if (volume_flag == .POSITIVE) {
		q.volume += step
	} else if (volume_flag == .NEGATIVE) {
		q.volume -= step
	}
	q.volume = rl.Clamp(q.volume, 0.0, 1.0)
	rl.SetMasterVolume(q.volume)
	return q
}
