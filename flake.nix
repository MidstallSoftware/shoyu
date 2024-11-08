{
  description = "A compositor framework for Flutter";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    systems.url = "github:nix-systems/default-linux";
    flake-utils.url = "github:numtide/flake-utils";
    nixos-apple-silicon = {
      url = "github:tpwrules/nixos-apple-silicon/release-2024-07-19";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    {
      self,
      nixpkgs,
      systems,
      flake-utils,
      nixos-apple-silicon,
      ...
    }:
    let
      inherit (nixpkgs) lib;
    in
    flake-utils.lib.eachSystem (import systems) (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system}.extend (
          pkgs: prev: with pkgs; {
            pkgsAsahi = (
              if stdenv.targetPlatform.isAarch64 then
                pkgsCross.aarch64-multiplatform.appendOverlays [
                  nixos-apple-silicon.overlays.default
                  (pkgsAsahi: prev: {
                    mesa-asahi-edge = prev.mesa-asahi-edge.overrideAttrs (
                      super: prev: {
                        meta = prev.meta // {
                          platforms = [
                            "i686-linux"
                            "x86_64-linux"
                            "x86_64-darwin"
                            "armv5tel-linux"
                            "armv6l-linux"
                            "armv7l-linux"
                            "armv7a-linux"
                            "aarch64-linux"
                            "powerpc64-linux"
                            "powerpc64le-linux"
                            "aarch64-darwin"
                            "riscv64-linux"
                          ];
                        };
                      }
                    );

                    mesa = if pkgsAsahi.targetPlatform.isAarch64 then pkgsAsahi.mesa-asahi-edge else prev.mesa;
                  })
                ]
              else
                null
            );
          }
        );
      in
      {
        devShells =
          let
            mkShell =
              pkgs:
              pkgs.mkShell {
                packages = with pkgs; [
                  pkg-config
                  gtk3
                  wlroots_0_18
                  meson
                  ninja
                  libdrm
                ];
              };
          in
          {
            default = mkShell pkgs;
          }
          // lib.optionalAttrs (pkgs.pkgsAsahi != null) { asahi = mkShell pkgs.pkgsAsahi; };
      }
    );
}
