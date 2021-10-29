
	.global	user_prog
	.set	user_prog,fact
fact:
	.fnstart
@ %bb.0:
	cmp	r0, #0
	moveq	r0, #1
	bxeq	lr
	sub	r1, r0, #1
	sub	r2, r0, #2
	umull	r2, r3, r1, r2
	mla	r0, r1, r1, r0
	and	r1, r3, #1
	lsrs	r1, r1, #1
	rrx	r1, r2
	add	r0, r0, #1
	sub	r0, r0, r1
	bx	lr
.Lfunc_end0:
	.size	fact, .Lfunc_end0-fact
	.cantunwind
	.fnend
