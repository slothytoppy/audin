package termaudio

import "core:fmt"
import "core:math"
import "core:os"
import "core:strings"
import "song_queue"
import rl "vendor:raylib"

SongQueue :: struct {
	paths:          song_queue.file_path_list,
	cursor:         u64,
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
	clone := strings.clone_to_cstring(path)
	defer delete(clone)
	q.music = rl.LoadMusicStream(clone)
	rl.PlayMusicStream(q.music)
	logger("log", "playing:", path)
	return q
}

/* NOTE: play_prev and play_next ensure that q.cursor 
   wraps around if you try to do q.cursor=0-1 or if you try to do q.cursor=len(q.paths.base_path)+1*/
@(require_results)
play_prev :: proc(q: SongQueue) -> SongQueue {
	q := q
	q.cursor = q.cursor > 0 ? q.cursor - 1 : q.paths.count - 1
	q = play_song(q.paths.fullpath[q.cursor], q)
	return q
}

@(require_results)
play_next :: proc(q: SongQueue) -> SongQueue {
	q := q
	q.cursor = q.cursor + 1 < cast(u64)len(q.paths.base_path) ? q.cursor + 1 : 0
	q = play_song(q.paths.fullpath[q.cursor], q)
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
