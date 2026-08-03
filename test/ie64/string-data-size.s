.section .data,"aw"
.local string_bytes
.type string_bytes,@object
string_bytes:
	dc.b "hello"
	ds.b 2
.size string_bytes,7
