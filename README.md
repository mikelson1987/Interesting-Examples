# Interesting-Examples

vector<ifstream> produce()
{
  vector<ifstream> ans;
  for (int i = 0;  i < 10; ++i) {
    ans.emplace_back(name(i));
  } 
  return ans;
}
 
void consumer()
{
  vector<ifstream> files = produce();
  for (ifstream& f: files) {
    read(f);
  }
} // close all files
