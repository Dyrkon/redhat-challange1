{
  lib,
  stdenv,
  ...
}:
let
  version = "0.1";
  pname = "mini-grep";
  srcPath = ./.;
in
stdenv.mkDerivation {
  inherit version pname srcPath;

  src = srcPath;

  enableParallelBuilding = true;

  nativeBuildInputs = [ stdenv.cc ];

  buildPhase = ''
    $CC -Wall -Wextra -Wpedantic -Werror -o ${pname} ${pname}.c
  '';
  installPhase = ''
    mkdir -p $out/bin
    mv ${pname} $out/bin
  '';

  meta = {
    description = "Custom implementation of ${lib.strings.removePrefix "mini-" pname}";
    license = lib.licenses.gpl3Plus;
    platforms = lib.platforms.all;
    mainProgram = pname;
  };
}
