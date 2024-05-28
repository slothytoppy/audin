package termaudio

import "base:runtime"
import "core:fmt"
import "core:log"
import "core:os"
import "core:strconv"
import "core:strings"

logger :: proc(file: string, args: ..any, sep := "") {
	fd, err := os.open(file, os.O_WRONLY | os.O_APPEND)
	if (err != os.ERROR_NONE) {
		fd, err = os.open(file, os.O_WRONLY | os.O_CREATE | os.O_APPEND, 0o644)
	}
	defer os.close(fd)
	for _, i in args {
		buf: [size_of(args[i])]byte
		if i > 0 {
			os.write_string(fd, sep)
		}
		switch v in args[i] {
		case string:
			os.write_string(fd, v)
		case byte:
			os.write_byte(fd, v)
		case f16:
		case f32:
		case f64:
			strconv.ftoa(buf[:], cast(f64)v, 'f', 0, 64)
		case bool:
			os.write_string(fd, strconv.itoa(buf[:], cast(int)v))
		case int:
			os.write_string(fd, strconv.itoa(buf[:], cast(int)v))
		case i32:
			os.write_string(fd, strconv.itoa(buf[:], cast(int)v))
		case i64:
			os.write_string(fd, strconv.itoa(buf[:], cast(int)v))
		case u16:
			os.write_string(fd, strconv.itoa(buf[:], cast(int)v))
		case u32:
			os.write_string(fd, strconv.itoa(buf[:], cast(int)v))
		case u64:
			os.write_string(fd, strconv.itoa(buf[:], cast(int)v))
		}
	}
}

init_logger :: proc() {
	log := context.logger
	//log.procedure = logger

}
