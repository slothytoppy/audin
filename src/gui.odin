package termaudio

import "core:fmt"
import "core:strconv"
import "core:strings"
import rl "vendor:raylib"

write_volume :: proc(q: SongQueue, pos: rl.Vector2) {
	buf: [8]byte
	str := strconv.ftoa(buf[:], cast(f64)q.volume, 'f', 1, 64)
	rl.DrawText(strings.clone_to_cstring(str[1:]), cast(i32)pos.x, cast(i32)pos.y, 36, rl.RED)
}

write_looping :: proc(looping: bool, pos: rl.Vector2 = {}) {
	msg: cstring = ""
	if (looping) {
		msg = "looping"
	}
	rl.DrawText(msg, cast(i32)pos.x, cast(i32)pos.y, 36, rl.GREEN)
}

keybinds: []rl.KeyboardKey = {.A, .D, .C, .V, .P, .L, .SPACE, .ESCAPE, .Q}

@(private)
is_key_pressed_or_held :: proc(key: rl.KeyboardKey) -> bool {
	if (rl.IsKeyPressed(key) || rl.IsKeyPressedRepeat(key)) {
		return true
	}
	return false
}

handle_keypress :: proc(key: rl.KeyboardKey, queue: SongQueue) -> SongQueue {
	q := queue
	if (is_key_pressed_or_held(.A)) {
		q = play_prev(q)
	}
	if (is_key_pressed_or_held(.D)) {
		q = play_next(q)
	}
	if (is_key_pressed_or_held(.C)) {
		q = change_volume(.NEGATIVE, q)
	}
	if (is_key_pressed_or_held(.V)) {
		q = change_volume(.POSITIVE, q)
	}
	if (rl.IsKeyPressed(.L)) {
		q.looping = !q.looping
	}

	return q
}
