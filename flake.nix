{
  description = "pqrv";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
  };

  outputs = { self, nixpkgs }: {
    packages.x86_64-linux.gcc14 = nixpkgs.legacyPackages.x86_64-linux.gcc-unwrapped.overrideAttrs (oldAttrs: rec {
      version = "14"; # Specify the desired version if available
    });
  };
}