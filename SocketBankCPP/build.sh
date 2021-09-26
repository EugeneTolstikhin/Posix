DIR="./bin"
if [ ! -d "$DIR" ]; then
  mkdir bin
fi

cd bin
g++ -std=c++11 -g -o admin_panel ../RAII.hpp ../RAIIArray.hpp ../FileRAII.hpp ../SocketBasic.h ../SocketBasic.cpp ../SocketClient.h ../SocketClient.cpp ../AdminPanel.cpp
g++ -std=c++11 -g -o points_generator ../RAII.hpp ../RAIIArray.hpp ../FileRAII.hpp ../SocketBasic.h ../SocketBasic.cpp ../SocketServer.h ../SocketServer.cpp ../PointsGenerator.cpp