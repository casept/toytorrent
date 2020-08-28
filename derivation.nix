{ stdenv, cmake, gtest, cpr, curlFull }:
stdenv.mkDerivation {
  name = "toytorrent";
  src = ./.;
  nativeBuildInputs = [ cmake curlFull.dev ];
  buildInputs = [
    cpr
    gtest
  ]; # gtest so integration that integration tests can be run in arion-compose network
  doCheck =
    false; # Integration tests are run in the arion-compose network, not here
  cmakeFlags = [ "-DBUILD_TESTING=ON" "-DINSTALL_TESTS=ON" ];
  checkInputs = [ gtest ];
}
