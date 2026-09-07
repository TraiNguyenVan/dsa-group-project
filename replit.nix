{ pkgs }: {
  deps = [
    pkgs.clang-tools
    pkgs.clang
    pkgs.gnumake
    pkgs.cmake
  ];
}