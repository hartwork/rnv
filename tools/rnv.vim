if exists("current_compiler")
  finish
endif
let current_compiler = "rnv"

let s:cpo_save = &cpo
set cpo-=C

setlocal makeprg=make\ -f\ validate.rnv\ %v
setlocal errorformat=error\ (%f\\,%l\\,%c):\ %m

let &cpo = s:cpo_save
unlet s:cpo_save
