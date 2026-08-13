# fence: every valid (predecessor, successor) pair, plus the bare `fence`
# alias and `fence.tso`.
#
# pred/succ sets are the 15 non-empty subsets of {i, o, r, w}.

# pred = w
fence w,w
fence w,r
fence w,rw
fence w,o
fence w,ow
fence w,or
fence w,orw
fence w,i
fence w,iw
fence w,ir
fence w,irw
fence w,io
fence w,iow
fence w,ior
fence w,iorw
# pred = r
fence r,w
fence r,r
fence r,rw
fence r,o
fence r,ow
fence r,or
fence r,orw
fence r,i
fence r,iw
fence r,ir
fence r,irw
fence r,io
fence r,iow
fence r,ior
fence r,iorw
# pred = rw
fence rw,w
fence rw,r
fence rw,rw
fence rw,o
fence rw,ow
fence rw,or
fence rw,orw
fence rw,i
fence rw,iw
fence rw,ir
fence rw,irw
fence rw,io
fence rw,iow
fence rw,ior
fence rw,iorw
# pred = o
fence o,w
fence o,r
fence o,rw
fence o,o
fence o,ow
fence o,or
fence o,orw
fence o,i
fence o,iw
fence o,ir
fence o,irw
fence o,io
fence o,iow
fence o,ior
fence o,iorw
# pred = ow
fence ow,w
fence ow,r
fence ow,rw
fence ow,o
fence ow,ow
fence ow,or
fence ow,orw
fence ow,i
fence ow,iw
fence ow,ir
fence ow,irw
fence ow,io
fence ow,iow
fence ow,ior
fence ow,iorw
# pred = or
fence or,w
fence or,r
fence or,rw
fence or,o
fence or,ow
fence or,or
fence or,orw
fence or,i
fence or,iw
fence or,ir
fence or,irw
fence or,io
fence or,iow
fence or,ior
fence or,iorw
# pred = orw
fence orw,w
fence orw,r
fence orw,rw
fence orw,o
fence orw,ow
fence orw,or
fence orw,orw
fence orw,i
fence orw,iw
fence orw,ir
fence orw,irw
fence orw,io
fence orw,iow
fence orw,ior
fence orw,iorw
# pred = i
fence i,w
fence i,r
fence i,rw
fence i,o
fence i,ow
fence i,or
fence i,orw
fence i,i
fence i,iw
fence i,ir
fence i,irw
fence i,io
fence i,iow
fence i,ior
fence i,iorw
# pred = iw
fence iw,w
fence iw,r
fence iw,rw
fence iw,o
fence iw,ow
fence iw,or
fence iw,orw
fence iw,i
fence iw,iw
fence iw,ir
fence iw,irw
fence iw,io
fence iw,iow
fence iw,ior
fence iw,iorw
# pred = ir
fence ir,w
fence ir,r
fence ir,rw
fence ir,o
fence ir,ow
fence ir,or
fence ir,orw
fence ir,i
fence ir,iw
fence ir,ir
fence ir,irw
fence ir,io
fence ir,iow
fence ir,ior
fence ir,iorw
# pred = irw
fence irw,w
fence irw,r
fence irw,rw
fence irw,o
fence irw,ow
fence irw,or
fence irw,orw
fence irw,i
fence irw,iw
fence irw,ir
fence irw,irw
fence irw,io
fence irw,iow
fence irw,ior
fence irw,iorw
# pred = io
fence io,w
fence io,r
fence io,rw
fence io,o
fence io,ow
fence io,or
fence io,orw
fence io,i
fence io,iw
fence io,ir
fence io,irw
fence io,io
fence io,iow
fence io,ior
fence io,iorw
# pred = iow
fence iow,w
fence iow,r
fence iow,rw
fence iow,o
fence iow,ow
fence iow,or
fence iow,orw
fence iow,i
fence iow,iw
fence iow,ir
fence iow,irw
fence iow,io
fence iow,iow
fence iow,ior
fence iow,iorw
# pred = ior
fence ior,w
fence ior,r
fence ior,rw
fence ior,o
fence ior,ow
fence ior,or
fence ior,orw
fence ior,i
fence ior,iw
fence ior,ir
fence ior,irw
fence ior,io
fence ior,iow
fence ior,ior
fence ior,iorw
# pred = iorw
fence iorw,w
fence iorw,r
fence iorw,rw
fence iorw,o
fence iorw,ow
fence iorw,or
fence iorw,orw
fence iorw,i
fence iorw,iw
fence iorw,ir
fence iorw,irw
fence iorw,io
fence iorw,iow
fence iorw,ior
fence iorw,iorw

# Bare fence (alias for fence iorw, iorw).
fence
# Fence with Total Store Ordering.
fence.tso
