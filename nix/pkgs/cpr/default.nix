{ stdenv, fetchFromGitHub, cmake, gtest, gnused, curl, zlib, static ? false }:

stdenv.mkDerivation rec {
  pname = "cpr";
  version = "1.6.2";

  src = fetchFromGitHub {
    owner = "whoshuu";
    repo = "cpr";
    rev = version;
    sha256 = "sha256-oV03Jcsi2Ja3ObQu7UW5sCF6Hx/7gb8axvcZuDULMAA=";
  };

  nativeBuildInputs = [ cmake ];
  buildInputs = [ curl zlib ];
  checkInputs = [ gtest ];
  doCheck =
    false; # Tries to pull in libs using cmake download which we don't have in nixpkgs
  cmakeFlags = [
    "-DCPR_FORCE_USE_SYSTEM_CURL=ON"
    "-DCPR_USE_SYSTEM_GTEST=ON"
    "-DCPR_BUILD_TESTS=OFF"
    "-DCPR_BUILD_TESTS_SSL=OFF"
    "-DBUILD_SHARED_LIBS=${if static then "OFF" else "ON"}"
  ];
  # For whatever reason, the .cmake file needed for other projects to find the lib is not installed
  patches = [ ./cpr-config.cmake.patch ];
  postInstall = ''
    cp ../cpr-config.cmake "$out/"
    substituteAllInPlace "$out/cpr-config.cmake"
  '';
  #--replace "CPR_INCLUDE_DIR" $out/include \
  meta = with stdenv.lib; {
    homepage = "whoshuu.github.io/cpr/";
    description =
      "A simple wrapper around libcurl for C++ inspired by the excellent Python Requests project";
    platforms = platforms.unix;
    license = licenses.mit;
  };
}
