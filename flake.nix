{
  description = "Dev shells for RISC-V 64: nixpkgs GNU toolchain (default) or XuanTie toolchain";
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
      # Prebuilt XuanTie toolchain (gcc, binutils, glibc incl. static libs).
      xuantie = pkgsHost.stdenvNoCC.mkDerivation {
        pname = "xuantie-900-gcc-linux-glibc";
        version = "3.0.2";
        src = pkgsHost.fetchurl {
          # Public mirror used by the official K230 Linux SDK (no registration).
          url = "https://download.kendryte.com/k230/downloads/dl/gcc/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V3.0.2-20250410.tar.gz";
          # sha256 of the tarball
          hash = "sha256-IhXh/+G5uSd7H1ry9hEC6DlzFjQE8psE2nCX/a+p4bc=";
        };
        nativeBuildInputs = [ pkgsHost.autoPatchelfHook ];
        # ncurses5: needed by the bundled simulator (riscv64-unknown-linux-gnu-run)
        buildInputs = [ pkgsHost.stdenv.cc.cc.lib pkgsHost.zlib pkgsHost.ncurses5 ];
        # Never strip: most of the tree is RISC-V target objects.
        dontStrip = true;
        installPhase = "mkdir -p $out && cp -a ./* $out/";
      };
    in
    {
      devShells.${system} = {
      # nix develop            -> your original nixpkgs cross toolchain
      default = pkgs.stdenv.mkDerivation
        {
          name = "RISC-V 64 Toolchain";
          nativeBuildInputs = [
            pkgsHost.qemu
          ];
          buildInputs = [
            pkgs.glibc.static
          ];
        };

      # nix develop .#xuantie  -> XuanTie toolchain
      xuantie = pkgsHost.stdenv.mkDerivation
        {
          name = "RISC-V 64 XuanTie Toolchain";
          nativeBuildInputs = [
            pkgsHost.qemu
            xuantie
          ];
        };
      };
    };
}