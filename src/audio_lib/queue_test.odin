package audio_lib

import "core:container/queue"
import "core:fmt"
import "core:testing"

@(test)
queue_test :: proc(t: ^testing.T) {
	q: queue.Queue(string)
	queue.init(&q)
	if (append_song(q, "../../stuff/Disturbed - Who.mp3") == false) {
		testing.fail(t)
	}
	fmt.println(queue.get(&q, 0))
}
