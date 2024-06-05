b() { cd build && { ninja; ninja_ec=$?; cd ..; return $ninja_ec; }; }
alias reconf="meson setup --reconfigure build ."
t() { b && build/src/mead; }
