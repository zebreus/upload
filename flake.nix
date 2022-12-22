{
  description = "Upload files to the internet";

  inputs = {
    nixpkgs.url = "nixpkgs/nixos-21.11";
    flake-utils.url = "github:numtide/flake-utils";
    # Start inserting submodules as inputs here
    cxxopts = {
      url = "https://github.com/jarro2783/cxxopts.git";
      flake = false;
      type = "git";
      rev = "aaa5e790b6b5b4c9de0b1ca3346ed0c2e55e4e03";
    };
    cpp-httplib = {
      url = "https://github.com/yhirose/cpp-httplib.git";
      flake = false;
      type = "git";
      rev = "ff813bf99d3402adafd4141b660ed2313d27f7a6";
    };
    miniz-cpp = {
      url = "https://github.com/Zebreus/miniz-cpp.git";
      flake = false;
      type = "git";
      rev = "5a7a6b2aae35cdd5aecb38b9fb51efd9178dd510";
    };
    # Stop inserting submodules as inputs here
  };

  outputs = { self, nixpkgs, flake-utils /* Insert submodules */, cxxopts, miniz-cpp, cpp-httplib /* Stop insert submodules */ }:
    flake-utils.lib.eachDefaultSystem (system:
      rec {
        name = "upload";
        packages.default = (with nixpkgs.legacyPackages.${system};
          stdenv.mkDerivation {
            name = name;
            src = ./.;

            nativeBuildInputs = [
              ronn
            ];

            buildInputs = [
              openssl
            ];

            patchPhase = ''
              # Start inserting submodule softlinks here
              rm -rf libs/cxxopts
              ln -s ${cxxopts} libs/cxxopts
              rm -rf libs/miniz-cpp
              ln -s ${miniz-cpp} libs/miniz-cpp
              rm -rf libs/cpp-httplib
              ln -s ${cpp-httplib} libs/cpp-httplib
              # Stop inserting submodule softlinks here
            '';

            buildPhase = ''
              export SOURCE_DATE_EPOCH="${toString self.lastModified}"
              make
            '';
          });

        devShells.default = (with nixpkgs.legacyPackages.${system};
          mkShell {
            nativeBuildInputs = packages.default.nativeBuildInputs ++ [
              upx
              clang-tools
              nixpkgs-fmt
              wget
            ];

            buildInputs = packages.default.buildInputs;
          });

        apps.default = flake-utils.lib.mkApp {
          drv = packages.default;
          exePath = "/bin/${name}";
        };
      }
    );
}
