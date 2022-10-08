#include <cmath>
#include <iostream>
#include "cpgplot.h"
#include <algorithm>

using namespace std;

float dC1dt(float C1, float k1, float a) {	
	return -(k1/(1+a*pow(C1,2)))*C1;
}

float dC2dt(float C1, float C2, float k1, float k2, float a) {
	return (k1/(1+a*pow(C1,2)))*C1 - k2*C2;
}

float dC3dt(float C2, float C3, float k2, float vmax, float km) {
	return k2*C2 - (vmax/(km+C3))*C3;
}

int main()
{
	float time;
	const int n = 512; //play around
	float t[n+1],C1[n+1], C2[n+1], C3[n+1],Ctot[n+1]; //time in hrs
	float vmax, km, V, k1, k2, a, D0, alcml, alcper, deltat;
	const float alcden =  0.7911; // g/ml at 37 degrees C = std stomach temp
	string gen;	

	cout << "This code models the intake and clearance of alcohol using the Pieter's 3-compartment model \n";
	cout << "For what gender do you want to model alcohol clearance? (Input one of M/F) \n";
	cin >> gen;

	if (gen == "M") { // set constants for men
		vmax = 0.47;
		km = 0.38;
		V = 38.4;
		k1 = 5.55; 
		k2 = 7.05;
		a = 0.42;
	}
	else if (gen == "F") { // set constants for women
		vmax = 0.48;
		km = 0.405;
		V = 28.5;
		k1 = 4.96; 
		k2 = 4.96;
		a = 0.75;
	}
	else if (gen != "M" or "F") { // gender has to be M or F
		cout << "Error: Gender must be one of 'M' or 'F'\n";
		return 1;
	}

	cout << "How long do you want to model alcohol clearance (in hrs)? \n"; // max time simulation runs
	cin >> time;

	if (time <= 0) {
		cout << "Error: time cannot be less than or equal to 0\n";
		return 1;
	} 	

	deltat = time/n; // max time length/n
	

	cout << "Please enter the amount of alcohol consumed (in mL) and the alcohol percentage at time t=0\n";
	cout << "For reference: \n1 standard can of beer = 340 mL at 5% \n1 standard glass of wine = 142 mL at 12% \n1 standard shot of liquor = 43 mL at 40%\n";
	cin >> alcml>>alcper;
	
	if (alcml <= 0) {
		cout<< "Error: alcohol volume cannot be zero or negative \n";
		return 1; 
	}
	if (alcper >100 or alcper <0) {
		cout << "Error: alcohol percentage must be between 0 and 100 \n";
		return 1;
	}
	if (alcper == 0) {
		cout << "Error: 0 percent alcohol isn't an alcoholic drink \n";
		return 1; 
	}


	D0 = alcml * (alcper/100) * alcden; // initial amount of alcohol in g

	C1[0] = D0/V; // initial conc in compartment 1 = stomach (g/L)
	C2[0] = 0; // initial conc in compartment 2 = small intestine
	C3[0] = 0; // initial conc in compartment 3 = central compartment

	string multidrink;
	int numdrinks;	
	
	cout << "Do you want to model more drinks over the course of "<<time<<" hours? (Y/N)\n";
	cin >> multidrink;

	// ----------------------------------------
	// Modelling more than 1 drink over t hours
	// ---------------------------------------- 

	if (multidrink == "Y") {
		cout <<"How many drinks over the course of "<<time<<" hours?\n";
		cin >> numdrinks; // number of drinks
		if (numdrinks <= 0) {
			cout << "Number of drinks cannot be less than 0\n"; 
			return 1;
		}
		float drinktime[numdrinks]; // the time of each later drink
		cout << "Please enter the time (in hrs after t=0) that drinks are consumed\n";
		for (int i = 0; i<numdrinks; i++) {
			cin >> drinktime[i];
		}
		
		// test for issues 
		for (int i = 0; i<numdrinks; i++) {
			if (drinktime[i] <= 0 || drinktime[i] > time) {
				cout<<"Error: the time of consuming a drink must be between 0 and "<<time<<'\n';
				return 1;
			}
		}
		
		string samedrink; 
		cout<< "Are each of the "<<numdrinks<<" drinks the same as the first? (Y/N)"<<'\n'<<" (i.e. "<<alcml<<" mL, "<<alcper<<" %)"<<'\n';
		cin>>samedrink;
		
		float alcml_m[numdrinks], alcper_m[numdrinks];
		if (samedrink == "Y") {
			for (int i = 0; i<numdrinks;i++) {
				alcml_m[i] = alcml;
				alcper_m[i] = alcper;
			}			
		}

		else if (samedrink == "N") {
			cout<<"Please input the volume in mL of the "<<numdrinks<<" drinks:\n";			
			for (int i = 0; i<numdrinks;i++) {
				cin>> alcml_m[i];
			}
			cout<<"Please input the alcohol percentage of the "<<numdrinks<<" drinks:\n";
			for (int i = 0; i<numdrinks;i++) {
				cin>> alcper_m[i];
			}
			// test for issues 
			for (int i = 0;i<numdrinks;i++) {
				if (alcml_m[i] < 0) {
					cout<<"Error: alcohol volume cannot be less than 0\n";
					return 1;
				}
				if (alcper_m[i] > 100 || alcper_m[i] <= 0) {
					cout<<"Error: acohol percentage must be between 0 and 100\n";
					return 1;
				}
			}
		}
		else if (samedrink != "Y" or "N") {
			cout<<"Error: response must be one of 'Y' or 'N'\n";
			return 1;
		}

		for (int i=0;i<=n+1;i++) {
			float alcg_m[numdrinks], C1g[numdrinks]; 
			int alcpt_m[numdrinks]; 
			for (int j = 0; j<numdrinks;j++) {
				alcg_m[j] = alcml_m[j] * (alcper_m[j]/100) * alcden; // grams of each drink
				alcpt_m[j] = (drinktime[j]/time)*n; // convert time to point on graph	
			
			t[i] = deltat*i;			

			for (int j = 0; j<numdrinks; j++) { // add alcohol at the times specified
				if (i == alcpt_m[j]) {
					C1g[j] = C1[alcpt_m[j]]*V; // grams of alc in C1 at time alcpt_m[i]
					C1[alcpt_m[j]] = (C1g[j] + alcg_m[j])/V; // grams of alcohol added + current grams / volume to get new conc of compartment 1 
				}
			} 			

			float k1C1 = deltat * dC1dt(C1[i], k1, a);
			float k2C1 = deltat * dC1dt(C1[i] + 0.5*k1C1, k1, a);
			float k3C1 = deltat * dC1dt(C1[i] + 0.5*k2C1, k1, a);
			float k4C1 = deltat * dC1dt(C1[i] + k3C1, k1, a);
			C1[i+1] = C1[i] + (k1C1 + 2*k2C1 + 2*k3C1 + k4C1)/6;

			float k1C2 = deltat * dC2dt(C1[i], C2[i], k1, k2, a);
			float k2C2 = deltat * dC2dt(C1[i] + 0.5*k1C1, C2[i] + 0.5*k1C2, k1, k2, a);
			float k3C2 = deltat * dC2dt(C1[i] + 0.5*k2C2, C2[i] + 0.5*k2C2, k1, k2, a);
			float k4C2 = deltat * dC2dt(C1[i] + k3C2, C2[i] + k3C2, k1, k2, a);
			C2[i+1] = C2[i] + (k1C2 + 2*k2C2 + 2*k3C2 + k4C2)/6;

			float k1C3 = deltat * dC3dt(C2[i], C3[i], k2, vmax, km);
			float k2C3 = deltat * dC3dt(C2[i] + 0.5*k1C3, C3[i] + 0.5*k1C3, k2, vmax, km);
			float k3C3 = deltat * dC3dt(C2[i] + 0.5*k2C3, C3[i] + 0.5*k2C3, k2, vmax, km);
			float k4C3 = deltat * dC3dt(C2[i] + k3C3, C3[i] + k3C3, k2, vmax, km);
			C3[i+1] = C3[i] + (k1C3 + 2*k2C3 + 2*k3C3 + k4C3)/6;
			
				
		}	
		
			
		}
	}

	else if (multidrink == "N") {
		for (int i=0;i<=n+1;i++) {	
			t[i] = deltat*i;
			float k1C1 = deltat * dC1dt(C1[i], k1, a);
			float k2C1 = deltat * dC1dt(C1[i] + 0.5*k1C1, k1, a);
			float k3C1 = deltat * dC1dt(C1[i] + 0.5*k2C1, k1, a);
			float k4C1 = deltat * dC1dt(C1[i] + k3C1, k1, a);
			C1[i+1] = C1[i] + (k1C1 + 2*k2C1 + 2*k3C1 + k4C1)/6;

			float k1C2 = deltat * dC2dt(C1[i], C2[i], k1, k2, a);
			float k2C2 = deltat * dC2dt(C1[i] + 0.5*k1C1, C2[i] + 0.5*k1C2, k1, k2, a);
			float k3C2 = deltat * dC2dt(C1[i] + 0.5*k2C2, C2[i] + 0.5*k2C2, k1, k2, a);
			float k4C2 = deltat * dC2dt(C1[i] + k3C2, C2[i] + k3C2, k1, k2, a);
			C2[i+1] = C2[i] + (k1C2 + 2*k2C2 + 2*k3C2 + k4C2)/6;

			float k1C3 = deltat * dC3dt(C2[i], C3[i], k2, vmax, km);
			float k2C3 = deltat * dC3dt(C2[i] + 0.5*k1C3, C3[i] + 0.5*k1C3, k2, vmax, km);
			float k3C3 = deltat * dC3dt(C2[i] + 0.5*k2C3, C3[i] + 0.5*k2C3, k2, vmax, km);
			float k4C3 = deltat * dC3dt(C2[i] + k3C3, C3[i] + k3C3, k2, vmax, km);
			C3[i+1] = C3[i] + (k1C3 + 2*k2C3 + 2*k3C3 + k4C3)/6;	
		}
	} 

	else if (multidrink != "Y" or "N") {
		cout << "Error: must enter one of 'Y' or 'N'\n";
		return 1;	 
	}
	
	for (int i=0; i<=n+1; i++) {
		Ctot[i] = C1[i] + C2[i] + C3[i];
	}
	

	// ------------------------------
	// Graphing alcohol concentration
	// -----------------------------

	if (!cpgopen("/XWINDOW")) return 1;

	cpgenv(0.,time,-0.01,5.,0,1);

	cpglab("Time (hrs)", "Concentration of alcohol in body (g/L)", "Alcohol concentration in the body over time after a drink at time t=0");


	cpgsci(4);
	cpgline(n+1,t,C1);

	cpgsci(8);
	cpgline(n+1,t,C2);
	
	cpgsci(12);
	cpgline(n+1,t,C3);
	
	cpgsci(9);
	cpgline(n+1,t,Ctot);

	cpgclos();

	// ---------------------------
	// Graphing BAC simple
	// ---------------------------
	
	float BAC[n+1];

	for (int i=0;i<=n+1;i++) {
		float galctot = C3[i]*V;
		BAC[i] = galctot/(500);
	}
	cout<<"BAC after "<<time<<" hours: "<<BAC[n+1]<<'\n';

	if (!cpgopen("/XWINDOW")) return 1;	

	cpgenv(0.,time,-0.01,0.5,0,1);

	cpglab("Time (hrs)", "BAC", "BAC over time after a drink at time t=0");
	
	cpgsci(9);
	cpgline(n+1,t,BAC);

	cpgclos();

	// ---------------------------
	// Graphing BAC individualized
	// ---------------------------
	
	float BACind[n+1];
	float height, weight, bloodvol;

	cout<<"Please input your height and weight in cm and kg, respectively \n";
	cin >> height>>weight;
	
	if (height <=0 or weight <=0) {
		cout << "Height or weight cannot be lower than 0\n"; 
		return 1;
	}
	
	// blood volume calculated using the Nadler equations 	
	
	if (gen == "M") {
		bloodvol = (0.3669*pow(height/100,3)) + (0.03219*weight) + 0.6041;		
	}
	else if (gen == "F") {
		bloodvol = (0.3561*pow(height/100,3)) + (0.03308*weight) + 0.1833;		
	}
	
	cout<<"Blood volume in L: "<<bloodvol<<'\n';

	for (int i=0;i<=n+1;i++) {
		float galctot = C3[i]*V;
		BACind[i] = galctot/(bloodvol*100);
	}
	cout<<"BAC after "<<time<<" hours: "<<BACind[n+1]<<'\n';

	if (!cpgopen("/XWINDOW")) return 1;	

	cpgenv(0.,time,-0.01,0.5,0,1);

	cpglab("Time (hrs)", "BAC", "Individualized BAC over time after a drink at time t=0");
	
	cpgsci(9);
	cpgline(n+1,t,BACind);

	cpgclos();
}
