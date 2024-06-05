package termaudio

import "./song_queue"
import "base:intrinsics"
import "core:fmt"
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

pos :: struct {
	x, y: i32,
}

textbox_color :: struct {
	box, text: rl.Color,
}

textbox :: proc(
	data: string,
	pos: pos,
	font: rl.Font,
	padding: u64 = 0,
	textbox_color: textbox_color,
) {
	msg := strings.clone_to_cstring(data)
	tb := textbox_color
	defer delete(msg)
	spacing: f32 = 0
	sz := rl.MeasureTextEx(font, msg, cast(f32)font.baseSize, spacing)
	width: i32 = cast(i32)sz.x + cast(i32)padding
	rl.DrawRectangleLines(pos.x, pos.y, width + 5, cast(i32)sz.y, tb.box)
	rl.DrawTextEx(font, msg, rl.Vector2{cast(f32)pos.x, cast(f32)pos.y}, sz.y, spacing, tb.text)
}

draw_int :: proc(
	pos: pos,
	data: $T,
	font: rl.Font,
	color: rl.Color,
) where intrinsics.type_is_numeric(T) {
	buf: [8]byte
	text: string = strconv.itoa(buf[:], cast(int)data)
	if (data < 10) {
		text = strings.concatenate({"0", text})
	}
	msg: cstring = strings.clone_to_cstring(text)
	sz := rl.MeasureTextEx(font, msg, cast(f32)font.baseSize, 0)
	height := sz.y
	rl.DrawTextEx(font, msg, rl.Vector2{cast(f32)pos.x, cast(f32)pos.y}, height, 0, color)
}
