{
  description = "A flake to set up a development shell with the RISC-V 64 GNU toolchain";
  inputs.nixpkgs.url = "nixpkgs/nixpkgs-unstable";
  outputs = { nixpkgs, ... }:
    let
      system = "x86_64-linux";
      # Host (x86_64) pkgs — for tools we run on the build machine
      # (e.g. the qemu user-mode emulator).
      pkgsHost = import nixpkgs { inherit system; };
      # Cross pkgs — for the RISC-V toolchain (gcc, binutils, glibc)
      # and anything the test binary links against.
      pkgs = import nixpkgs {
          inherit system;
          crossSystem = {
            config = "riscv64-unknown-linux-gnu";
          };
        };
    in
    with pkgs;
    {
      devShells.${system}.default = stdenv.mkDerivation
        {
          name = "RISC-V 64 Toolchain";
          nativeBuildInputs = [
            pkgsHost.qemu
          ];
          buildInputs = [
            glibc.static
          ];
        };
    };
}