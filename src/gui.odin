package termaudio

import "./song_queue"
import "core:strconv"
import "core:strings"
import rl "vendor:raylib"

Position :: struct {
	x, y, w, h: i32,
}

Button_State :: enum {
	hover,
	clicked,
}

write_volume :: proc(q: SongQueue, pos: rl.Vector2) {
	buf: [8]byte
	str := strconv.ftoa(buf[:], cast(f64)q.volume, 'f', 1, 64)
	rl.DrawText(strings.clone_to_cstring(str[1:]), cast(i32)pos.x, cast(i32)pos.y, 36, rl.RED)
}

write_looping :: proc(looping: bool, pos: rl.Vector2 = {}) {
	msg: cstring = ""
	if (looping) {
		msg = "looping"
		rl.DrawText(msg, cast(i32)pos.x, cast(i32)pos.y, 36, rl.GREEN)
	}
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
	if (rl.IsKeyPressed(.M)) {
		q = toggle_mute(q)
	}
	if (rl.IsKeyPressed(.P)) {
		q = toggle_playing(q)
	}
	if (rl.IsKeyPressed(.L)) {
		/*
		fi := song_queue.read_dir("./extras")
		rl.TextFormat("hello", q.paths.files)
		i: u64 = 0
		for ; i < fi.count; i += 1 {
			append(&q.paths.base_path, fi.files[i])
		}
		q.paths.count = q.paths.count + i
    */
	}

	return q
}

make_button :: proc(
	pos: rl.Rectangle,
	text: string = "",
	color: rl.Color,
) -> (
	state: Button_State,
) {
	rl.DrawRectangle(
		cast(i32)pos.x,
		cast(i32)pos.y,
		cast(i32)pos.width,
		cast(i32)pos.height,
		color,
	)
	font := rl.GetFontDefault()
	font_size := font.baseSize
	rl.DrawText(
		strings.clone_to_cstring(text),
		cast(i32)pos.x,
		cast(i32)pos.height,
		font_size,
		rl.GREEN,
	)
	if (rl.CheckCollisionPointRec(rl.GetMousePosition(), pos)) {
		if (rl.IsMouseButtonPressed(.LEFT)) {
			state = .clicked
		} else {
			state = .hover
		}
	}
	return state
}
