{ stdenv, cmake, pkg-config, gtest, cpr, curlFull, botan2, fmt, boost175, opentracker, aria2 }:
stdenv.mkDerivation {
  name = "toytorrent";
  src = ./.;
  nativeBuildInputs = [ cmake curlFull.dev pkg-config ];
  buildInputs = [
    cpr
    botan2
    fmt
  ];
  doCheck = true;
  cmakeFlags = [ "-DBUILD_TESTING=ON" ];
  checkInputs = [ gtest boost175 opentracker aria2 ];
}
