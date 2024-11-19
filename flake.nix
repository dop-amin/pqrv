{
  description = "A flake to set up a development shell with the RISC-V 64 GNU toolchain";

  inputs.nixpkgs.url = "nixpkgs/nixpkgs-unstable";

  outputs = { nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs {
          system = "x86_64-linux";
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
          ];

          buildInputs = [
            glibc.static
          ];
        };
    };
}