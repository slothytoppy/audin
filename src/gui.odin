package termaudio

import "core:strconv"
import "core:strings"
import rl "vendor:raylib"

write_volume :: proc(q: SongQueue, pos: rl.Vector2, loc := #caller_location) {
	buf: [8]byte
	rl.DrawText(
		strings.clone_to_cstring(strconv.ftoa(buf[:], cast(f64)q.volume, 'f', 1, 64)),
		cast(i32)pos.x,
		cast(i32)pos.y,
		36,
		rl.RED,
	)
}
