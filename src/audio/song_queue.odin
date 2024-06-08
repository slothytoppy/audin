package audio

import "base:builtin"

get_queue_max_len :: proc(q: file_path_list) -> u64 {
	longest: u64 = 0
	for i in 0 ..< len(q) {
		curr_len: u64 = cast(u64)builtin.len(base_name(q, i))
		if (curr_len > longest) {
			longest = curr_len
		}
	}
	return longest
}
