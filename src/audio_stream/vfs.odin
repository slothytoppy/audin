package audio_stream

import ma "vendor:miniaudio"

Vfs :: struct {
	data: ma.data_source_base,
}

init :: proc() {
	vfs: Vfs
	dscg: ma.data_source_config
	ds: ma.data_source
	ma.data_source_init(&dscg, &ds)
}
