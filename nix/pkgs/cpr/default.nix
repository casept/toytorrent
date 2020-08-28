{ stdenv, fetchFromGitHub, cmake, gtest, gnused, curl, static ? false }:

stdenv.mkDerivation rec {
  pname = "cpr";
  version = "1.5.1";

  src = fetchFromGitHub {
    owner = "whoshuu";
    repo = "cpr";
    rev = "v" + version;
    sha256 = "17xackp3kcr23vgfykyhbzpgd53jhbsc3my14221v5wmz6g6lvng";
  };

  nativeBuildInputs = [ cmake ];
  buildInputs = [ curl ];
  checkInputs = [ gtest ];
  doCheck =
    false; # Tries to pull in libs using cmake download which we don't have in nixpkgs
  cmakeFlags = [
    "-DUSE_SYSTEM_CURL=ON"
    "-DUSE_SYSTEM_GTEST=ON"
    "-DBUILD_CPR_TESTS=OFF"
    "-DBUILD_CPR_TESTS_SSL=OFF"
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
