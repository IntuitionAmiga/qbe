.section .data,"aw"
.local answer
.type answer,@object
.align 8
answer:
	dc.q 42
	ds.b 8
.size answer,16
