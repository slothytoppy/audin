package termaudio

import "base:runtime"
import "core:fmt"
import "core:log"
import "core:os"
import "core:reflect"
import "core:strconv"
import "core:strings"

logger :: proc(
	file: string,
	args: ..any,
	sep := "",
	newline: bool = true,
	loc := #caller_location,
) {
	fd, err := os.open(file, os.O_WRONLY | os.O_APPEND)
	if (os.exists(file) == false) {
		fd, err = os.open(file, os.O_WRONLY | os.O_CREATE | os.O_APPEND, 0o644)
	}
	defer os.close(fd)
	for _, i in args {
		switch v in args[i] {
		case string, cstring:
			fmt.fprint(fd, v, sep)
		case byte:
			fmt.fprint(fd, v, sep)
		// fmt.fprint(fd, v, sep)
		case f16, f32, f64:
			fmt.fprint(fd, v, sep)
		// fmt.fprint(fd, v, sep)
		case bool:
			fmt.fprint(fd, v, sep)
		// fmt.fprint(fd, v, sep)
		case int, i32, i64:
			fmt.fprint(fd, v, sep)
		// fmt.fprint(fd, v, sep)
		case u16, u32, u64:
			fmt.fprint(fd, v, sep)
		// fmt.fprint(fd, v, sep)
		case:
			fmt.fprint(fd, "unknown type: value", v, "at:", loc, flush = false)
		// fmt.fprint_type(fd, info)
		}
	}
	if (newline == true) {
		os.write_string(fd, "\n")
	}
}
