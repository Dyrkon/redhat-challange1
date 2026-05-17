{
  description = "Flake for Red Hat challenges";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs }: 
  let
    pkgs = systemArch: import nixpkgs {
          system = systemArch;
          config = {
            allowUnfree = true;
            cudaSupport = pkgs.stdenv.isLinux;
          };
        };
    lib = systemArch: (pkgs systemArch).lib;
    stdenv = systemArch: (pkgs systemArch).stdenv;
    packages = systemArch: {
      mini-grep = import ./mini-grep.nix { inherit (pkgs systemArch) lib stdenv; };
      mini-ls = import ./mini-ls.nix { inherit (pkgs systemArch) lib stdenv; };
    };
  in
  {
    packages.aarch64-darwin = packages "aarch64-darwin";
    packages.x86_64-linux = packages "x86_64-linux";
  };
}

