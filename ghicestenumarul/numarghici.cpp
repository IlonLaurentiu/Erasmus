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
    int nearest_low = 1, nearest_high = 100;
cout<<"******************"<<endl;
cout<<"*Guess the number*"<<endl;
cout<<"******************"<<endl;
cout<<endl<< "Acest joc genereaza un numar random ."<<endl<<"Tu ai 10 incercari sa il ghicesti, "<<endl<<"iar la fiecare gresala primesti un hint"<<endl;
do{
    cout <<endl<< "Enter your guessed number ";
    cin>>n;
    k--;
    if(n>num)
    {cout<<"     to big    "<<k<<" attempts left"<<endl;
    nearest_high = n;}
    else  if(n<num)
    {cout<<"     to small    "<<k<<" attempts left"<<endl;
    nearest_low = n;}
    else
    cout<<"good, you did it in "<<k<<" moves";
    if(k==0)
    cout<<"you lost the number was "<<num<<endl;
    else if(n!=num)
    cout<<"The number is between "<<nearest_low<<" and "<<nearest_high<<endl;
} while( n!=num && k!=0 );
}