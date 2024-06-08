package gui

import "../audio"
import "base:intrinsics"
import "core:fmt"
import "core:strconv"
import "core:strings"
import rl "vendor:raylib"

Position :: struct {
	x, y, w, h: i32,
}

window_state: struct {
	fullscreen:    bool,
	width, height: i32,
}

Button_State :: enum {
	hover,
	clicked,
}

write_volume :: proc(q: audio.SongQueue, pos: rl.Vector2) {
	buf: [8]byte
	str := strconv.ftoa(buf[:], cast(f64)q.volume, 'f', 1, 64)
	// str[1:] is to get rid of the +/- sign for floats that odin does for me
	rl.DrawText(strings.clone_to_cstring(str[1:]), cast(i32)pos.x, cast(i32)pos.y, 36, rl.RED)
}

@(private)
is_key_pressed_or_held :: proc(key: rl.KeyboardKey) -> bool {
	if (rl.IsKeyPressed(key) || rl.IsKeyPressedRepeat(key)) {
		return true
	}
	return false
}

handle_keypress :: proc(key: rl.KeyboardKey, queue: audio.SongQueue) -> audio.SongQueue {
	q := queue
	if (is_key_pressed_or_held(.A)) {
		q = audio.play_prev(q)
	}
	if (is_key_pressed_or_held(.D)) {
		q = audio.play_next(q)
	}
	if (is_key_pressed_or_held(.C)) {
		q = audio.change_volume(.NEGATIVE, q)
	}
	if (is_key_pressed_or_held(.V)) {
		q = audio.change_volume(.POSITIVE, q)
	}
	if (rl.IsKeyPressed(.M)) {
		q = audio.toggle_mute(q)
	}
	if (rl.IsKeyPressed(.P)) {
		q = audio.toggle_playing(q)
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

/*
this is because only need to use one font and not render text in different fonts based on something
*/
@(private)
curr_font: rl.Font = rl.GetFontDefault()

set_font :: proc(font: rl.Font) {
	curr_font = font
}

textbox :: proc(data: string, pos: pos, maximum_width: u64 = 0, textbox_color: textbox_color) {
	msg := strings.clone_to_cstring(data)
	textbox := textbox_color
	defer delete(msg)
	spacing: f32 = 0
	sz := rl.MeasureTextEx(curr_font, msg, cast(f32)curr_font.baseSize, spacing)
	//fmt.println(cast(int)curr_font.baseSize * len(msg) / cast(int)curr_font.glyphPadding)
	width: i32 = cast(i32)maximum_width * curr_font.baseSize
	tmp_str := strings.clone_to_cstring(strings.repeat("a", cast(int)maximum_width))
	box_size := rl.MeasureTextEx(curr_font, tmp_str, cast(f32)curr_font.baseSize, 0)
	rl.DrawRectangleLines(0, pos.y, cast(i32)box_size.x, cast(i32)box_size.y, textbox.box)
	rl.DrawTextEx(curr_font, msg, rl.Vector2{cast(f32)0, cast(f32)pos.y}, 32, 0, textbox.text)
}

draw_int :: proc(data: $T, pos: pos, color: rl.Color) where intrinsics.type_is_numeric(T) {
	buf: [8]byte
	text: string = strconv.itoa(buf[:], cast(int)data)
	if (data < 10) {
		text = strings.concatenate({"0", text})
	}
	msg: cstring = strings.clone_to_cstring(text)
	sz := rl.MeasureTextEx(curr_font, msg, cast(f32)curr_font.baseSize, 0)
	height := sz.y
	rl.DrawTextEx(curr_font, msg, rl.Vector2{cast(f32)pos.x, cast(f32)pos.y}, height, 0, color)
	delete(msg)
}

draw_float :: proc(data: $T, pos: pos, color: rl.Color) where intrinsics.type_is_float(T) {
	buf: [8]byte
	text: string = strconv.ftoa(buf[:], cast(f64)data, 'f', 1, 64)
	text = text[1:]
	msg: cstring = strings.clone_to_cstring(text)
	sz := rl.MeasureTextEx(curr_font, msg, cast(f32)curr_font.baseSize, 0)
	height := sz.y
	rl.DrawTextEx(curr_font, msg, rl.Vector2{cast(f32)pos.x, cast(f32)pos.y}, height, 0, color)
	delete(msg)
}
