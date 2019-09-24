#include<iostream>
#include<vector>
#include<string>
#include<cstring>
using namespace std;
void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
  std::string::size_type pos1, pos2;
  pos2 = s.find(c);
  pos1 = 0;
  while(std::string::npos != pos2)
  {
    v.push_back(s.substr(pos1, pos2-pos1));

    pos1 = pos2 + c.size();
    pos2 = s.find(c, pos1);
  }
  if(pos1 != s.length())
    v.push_back(s.substr(pos1));
}


int main()
{
    vector<string> ver;
    string test_buf = "aaa*bbb*ccc*ddd*eee";
    SplitString(test_buf,ver,"*");
    for(auto &k : ver)
    {
        cout<<k<<" ";
    }

}