package termaudio

import c "core:c"
import rl "vendor:raylib"

open_window :: proc(x: i32 = 720, y: i32 = 480, window_name: cstring) {
	rl.InitWindow(x, y, window_name)
}

close_window :: proc() {
	rl.CloseWindow()
}

should_gui_close :: proc() -> bool {
	return rl.WindowShouldClose()
}

set_background :: proc(color: rl.Color) {
	rl.ClearBackground(color)
}

get_key_pressed :: proc() -> rl.KeyboardKey {
	return rl.GetKeyPressed()
}

is_key_down :: proc(key: rl.KeyboardKey) -> bool {
	return rl.IsKeyDown(key)
}

is_key_pressed :: proc(key: rl.KeyboardKey) -> bool {
	return rl.IsKeyPressed(key)
}

is_key_pressed_repeatedly :: proc(key: rl.KeyboardKey) -> bool {
	return rl.IsKeyPressedRepeat(key)
}

is_key_released :: proc(key: rl.KeyboardKey) -> bool {
	return rl.IsKeyReleased(key)
}

write_text :: proc(text: cstring, font: rl.Font, x, y: c.int, color: rl.Color) {
	rl.DrawText(text, x, y, 12, color)
}
