{
  description = "A compositor framework for Flutter";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/release-24.11";
    systems.url = "github:nix-systems/default-linux";
    flake-utils.url = "github:numtide/flake-utils";
    nixos-apple-silicon = {
      url = "github:tpwrules/nixos-apple-silicon/release-2024-12-25";
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

      defaultOverlay =
        pkgs: prev: with pkgs; {
          shoyu = stdenv.mkDerivation (finalAttrs: {
            pname = "shoyu";
            version = self.shortRev or "dirty";

            outputs = [
              "out"
              "dev"
              "devdoc"
            ];

            src = lib.cleanSource self;

            nativeBuildInputs = with pkgs; [
              ninja
              meson
              python3
              gobject-introspection
              gi-docgen
              pkg-config
              vala
              wayland-scanner
            ];

            buildInputs = finalAttrs.propagatedBuildInputs ++ [
              libxkbcommon
              mesa
              udev
              libepoxy
            ];

            propagatedBuildInputs = [
              glib
              wlroots_0_18
              gtk3
              gtk4
            ];

            mesonFlags = [
              (lib.mesonBool "documentation" true)
            ];

            postPatch = ''
              files=(
                build-aux/meson/gen-visibility-macros.py
              )

              chmod +x ''${files[@]}
              patchShebangs ''${files[@]}
            '';

            postFixup = ''
              moveToOutput "share/doc" "$devdoc"
            '';
          });
        };
    in
    flake-utils.lib.eachSystem (import systems) (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system}.appendOverlays [
          (
            pkgs: prev: with pkgs; {
              pkgsAsahi = (
                if stdenv.hostPlatform.isAarch64 then
                  pkgs.appendOverlays [
                    nixos-apple-silicon.overlays.default
                    (pkgsAsahi: prev: {
                      mesa = pkgsAsahi.mesa-asahi-edge;
                    })
                  ]
                else
                  null
              );
            }
          )
          defaultOverlay
        ];
      in
      {
        packages =
          {
            default = pkgs.shoyu;
          }
          // lib.optionalAttrs (pkgs.pkgsAsahi != null) {
            asahi = pkgs.pkgsAsahi.shoyu;
          };

        devShells =
          {
            default = pkgs.shoyu;
          }
          // lib.optionalAttrs (pkgs.pkgsAsahi != null) {
            asahi = pkgs.pkgsAsahi.shoyu;
          };

        legacyPackages = pkgs;
      }
    )
    // {
      overlays = {
        default = defaultOverlay;
        shoyu = defaultOverlay;
      };
    };
}
