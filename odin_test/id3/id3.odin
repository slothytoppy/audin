package id3

import "core:fmt"
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

@(test)
print_half :: proc() {
	halve :: proc(n: int) -> Maybe(int) {
		if n % 2 != 0 do return nil
		return n / 2
	}

	half, ok := halve(2).?
	if ok do fmt.println(half) // 1
	half, ok = halve(3).?
	if !ok do fmt.println("3/2 isn't an int")

	n := halve(4).? or_else 0
	fmt.println(n) // 2

}

parse_song :: proc(song_name: string) -> id3_info {
	fmt.assertf(
		"figure out id3 structure by reading https://mutagen-specs.readthedocs.io/en/latest/id3/id3v2.4.0-structure.html" ==
		"",
		"figure out id3 structure by reading https://mutagen-specs.readthedocs.io/en/latest/id3/id3v2.4.0-structure.html",
	)
	id3 := id3_info{}
	id3_enum := id3_enum{}
	data, err := os.read_entire_file_from_filename(song_name)
	fd, _ := os.open("log", os.O_WRONLY | os.O_APPEND)
	os.write(fd, data[:256])
	defer (os.close(fd))
	cursor: u64 = 10
	header := strings.clone_from_bytes(data[:3])
	if (header != "ID3") {
		id3.error_info.err = .PARSE_ERR
		return id3
	}
	fmt.printfln("%s", data[0:9])
	id3.error_info.err = .OK
	return id3
}
