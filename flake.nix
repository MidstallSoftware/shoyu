{
  description = "A compositor framework for Flutter";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    systems.url = "github:nix-systems/default-linux";
    flake-utils.url = "github:numtide/flake-utils";
    nixos-apple-silicon = {
      url = "github:tpwrules/nixos-apple-silicon/release-2024-11-12";
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

            shoyu = stdenv.mkDerivation {
              pname = "shoyu";
              version = self.shortRev or "dirty";

              outputs = [ "out" "dev" "devdoc" ];

              src = lib.cleanSource self;

              nativeBuildInputs = with pkgs; [
                ninja
                meson
                python3
                gobject-introspection
                gi-docgen
                pkg-config
                vala
              ];

              buildInputs = [
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
            };
          }
        );
      in
      {
        packages = {
          default = pkgs.shoyu;
          llvm = pkgs.pkgsLLVM.shoyu;
        };

        devShells = {
            default = pkgs.shoyu;
            llvm = pkgs.pkgsLLVM.shoyu;
          }
          // lib.optionalAttrs (pkgs.pkgsAsahi != null) {
            asahi = pkgs.pkgsAsahi.shoyu;
            asahi-llvm = pkgs.pkgsLLVM.pkgsAsahi.shoyu;
          };
      }
    );
}
