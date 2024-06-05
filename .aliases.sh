b() { cd build && { ninja; cd .. }; }
alias reconf="meson setup --reconfigure build ."
t() { b && build/src/mead; }
