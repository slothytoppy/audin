package termaudio

import "core:os"
import "core:strings"
import "song_queue"
import rl "vendor:raylib"

SongQueue :: struct {
	using paths:          song_queue.file_path_list,
	cursor:         i64,
	muted, playing: bool,
	volume:         f32,
	music:          rl.Music,
}

toggle_playing :: proc(q: SongQueue) -> SongQueue {
	q := q
	q.playing = !q.playing
	if (q.playing == false) {
		rl.PauseMusicStream(q.music)
	} else {
		rl.ResumeMusicStream(q.music)
	}
	return q
}

toggle_mute :: proc(q: SongQueue) -> SongQueue {
	q := q
	q.muted = !q.muted
	if (q.muted) {
		rl.SetMasterVolume(0)
	} else {
		rl.SetMasterVolume(q.volume)
	}
	return q
}

@(require_results)
play_song :: proc(path: string, q: SongQueue) -> SongQueue {
	q := q
	assert(os.exists(path))
	q.music = rl.LoadMusicStream(strings.clone_to_cstring(path))
	rl.PlayMusicStream(q.music)
	logger("log", "playing:", path)
	return q
}

@(require_results)
play_prev :: proc(q: SongQueue) -> SongQueue {
	q := q
	q.cursor -= 1
	if (q.cursor < 0) {
		q.cursor = cast(i64)q.paths.count - 1
	}
	q = play_song(q.paths.files[q.cursor], q)
	return q
}

@(require_results)
play_next :: proc(q: SongQueue) -> SongQueue {
	q := q
	q.cursor += 1
	if (q.cursor + 1 > cast(i64)q.paths.count) {
		q.cursor = 0
	}
	q = play_song(q.paths.files[q.cursor], q)
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
