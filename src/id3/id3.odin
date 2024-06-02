package id3

import "core:os"
import "core:strings"

id3_enum :: enum {
	OK,
	ERR,
	ENOENT,
	PARSE_ERR,
}

id3_error_info :: struct {
	err:  id3_enum,
	line: u64,
	col:  u64,
	file: string,
}

id3_info :: struct {
	song_name, album, artist: string,
	error_info:               id3_error_info,
}

todo :: proc(procedure := "", loc := #caller_location) {
	// fmt.panicf("todo: %s", procedure, loc)
	unimplemented(procedure, loc)
	// panic(procedure, loc)
	//	assert(false, "", loc)
}

@(require_results)
parse_song :: proc(song_name: string) -> id3_info {
	id3 := id3_info{}
	id3_enum := id3_enum{}
	data, err := os.read_entire_file_from_filename(song_name)
	if (string(data[:3]) != "ID3") {
		id3.error_info.err = .PARSE_ERR
		return id3
	}
	cursor: u64 = 3
	should_continue := true
	found_tag := false
	if (cursor > 0) {
		for i := 0; i < 196; i += 1 {
			if (strings.compare(string(data[i:i + len("ID3")]), "ID3") == 0) {
				i += 3
			}
			if (strings.compare(string(data[i:i + len("TIT2")]), "TIT2") == 0) {
				i += 3
			}
			if (strings.compare(string(data[i:i + len("APIC")]), "APIC") == 0) {
				break
			}
			//	if (data[i] > 65 && data[i] < 128) {fmt.printf("%d ", data[i])}
		}
		return id3
	}
	for ; cursor <= 256; cursor += 1 {
		if (found_tag == false &&
			   cursor < cast(u64)len(data) - 4 &&
			   strings.compare(string(data[cursor:cursor + 4]), "TIT2") == 0) {
			cursor += 4
			found_tag = true
		}
		if (strings.compare(string(data[cursor:cursor + len("APIC")]), "APIC") == 0) {
			cursor += 4
			should_continue = false
			break
		} else if (found_tag == true && data[cursor] > 65 && data[cursor] <= 126) {
			//	fmt.printf("%c", data[cursor])
		}
	}
	//fmt.printf("\n")
	// fmt.printfln("end in bytes %d %c", cursor, data[cursor])
	id3.error_info.err = .OK
	return id3
}
