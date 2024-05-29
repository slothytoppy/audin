package termaudio

import rl "vendor:raylib"

SongQueue :: struct {
	files:          rl.FilePathList,
	cursor:         i64,
	muted, playing: bool,
	volume:         f32,
	music:          rl.Music,
}

toggle_pause :: proc(q: SongQueue) {
	q := q
	q.playing = !q.playing
	if (q.playing == true) {
		rl.PauseMusicStream(q.music)
	} else {
		rl.ResumeMusicStream(q.music)
	}
}

@(require_results)
play_song :: proc(q: SongQueue) -> SongQueue {
	q := q
	q.music = rl.LoadMusicStream(q.files.paths[q.cursor])
	rl.PlayMusicStream(q.music)
	logger("log", "playing:", q.files.paths[q.cursor])
	return q
}

@(require_results)
play_prev :: proc(q: SongQueue) -> SongQueue {
	q := q
	q.cursor -= 1
	if (q.cursor < 0) {
		q.cursor = cast(i64)q.files.count - 1
	}
	q = play_song(q)
	return q
}

@(require_results)
play_next :: proc(q: SongQueue) -> SongQueue {
	q := q
	q.cursor += 1
	if (q.cursor + 1 >= cast(i64)q.files.count) {
		q.cursor = 0
	}
	q = play_song(q)
	return q
}
