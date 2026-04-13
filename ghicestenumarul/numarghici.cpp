#include <iostream>
#include <random>
using namespace std;

int main ()
{ 
int n,k=10;
    std::random_device rd; 
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);
    int num = dist(gen);
    std::cout << num << std::endl;

do{
    cout << "Introduce el numero a adivinar ";
    cin>>n;
    k--;
    if(n>num)
    cout<<"to big "<<k<<" attempts left"<<endl;
    else  if(n<num)
    cout<<"to small "<<k<<" attempts left"<<endl;
    else
    cout<<"good, you did it in "<<k<<" moves";
    if(k==0)
    cout<<"you lost"<<endl;
} while( n!=num && k!=0 );
}