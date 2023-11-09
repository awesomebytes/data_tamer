#include "data_tamer/data_tamer.hpp"
#include "data_tamer/sinks/mcap_sink.hpp"
#include <iostream>

int main()
{
  using namespace DataTamer;

  // start defining one or more Sinks that must be added by default.
  // Dp ot BEFORE creating a channel.
  auto mcap_sink = std::make_shared<MCAPSink>("test_1M.mcap");
  ChannelsRegistry::Global().addDefaultSink(mcap_sink);

  // Create (or get) a channel using the global registry (singleton)
  auto channel = ChannelsRegistry::Global().getChannel("chan");

  auto const vect_size = 250;
  std::vector<double> vect64(vect_size);
  std::vector<float> vect32(vect_size);
  std::vector<float> int32(vect_size);
  std::vector<float> int16(vect_size);

  long time_diff_nsec = 0;

  for(size_t i=0; i<vect_size; i++)
  {
    auto num = std::to_string(i);
    channel->registerValue( std::string("vect64_") + num, &vect64[i]);
    channel->registerValue( std::string("vect32_") + num, &vect32[i]);
    channel->registerValue( std::string("int32_") + num, &int32[i]);
    channel->registerValue( std::string("int16_") + num, &int16[i]);
  }

  long count = 0;

  double t = 0;
  while(count < 10*1000) // 10 simulated seconds
  {
    auto S = std::sin(t);
    for(size_t i=0; i<vect_size; i++)
    {
      vect64[i] = i + S;
      vect32[i] = i + S;
      int32[i] = int( 10*(i+S) );
      int16[i] = int( 10*(i+S) );
    }

    if( count++ % 1000 == 0)
    {
      std::cout << t << std::endl;
    }
    auto t1 = std::chrono::system_clock::now();
    if(!channel->takeSnapshot())
    {
      std::cout << "pushing failed\n";
    }
    auto t2 = std::chrono::system_clock::now();

    auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1);
    time_diff_nsec += diff.count();

    std::this_thread::sleep_for(std::chrono::microseconds(100));
    t += 0.001;
  }
  std::cout << "average execution time of takeSnapshot(): "
            << time_diff_nsec/count << " nanoseconds" << std::endl;
}