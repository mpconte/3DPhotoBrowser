/**
 * IndexSorter
 * Use the image index file to generate a new sorted file--used by the 3DPhotoBrowser
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
using namespace std;

/**
 * IndexFileImageData
 * The format of each image in the index file
 */
struct IndexFileImageData
{
	bool operator<(const IndexFileImageData& in_Other)
	{
		if(Year < in_Other.Year)
			return true;
		if(Year > in_Other.Year)
			return false;

		if(DayOfYear < in_Other.DayOfYear)
			return true;
		if(DayOfYear > in_Other.DayOfYear)
			return false;

		if(TimeOfDay < in_Other.TimeOfDay)
			return true;
		if(TimeOfDay > in_Other.TimeOfDay)
			return false;

		return false;
	}

	int				Width;
	int				Height;
	unsigned		TimeOfDay;
	short			DayOfYear;
	short			Year;
	unsigned char	AverageRed;
	unsigned char	AverageGreen;
	unsigned char	AverageBlue;
	int				FolderIndex;
	char			Filename[256];

	struct
	{
		unsigned ThumbFileOffset;
		unsigned ThumbContainerIndex;
		unsigned ThumbImageSize;
	} Thumbnails[6];
};

bool ParseIndexFile(const char* in_File);
bool WriteSortedIndexFile(const char* in_File);
bool WriteIndexCountFile(const char* in_File);

vector<IndexFileImageData> gImageData;
map<short, vector<unsigned>> gDayCounts;

int main(int argc, char* argv[])
{
	cout << "Sorting..." << endl;
	const char* l_IndexFileName = "../3DPhotoBrowser/Binaries/data/photo_index.dat";
	if( ParseIndexFile(l_IndexFileName) && WriteSortedIndexFile(l_IndexFileName) )
	{
		cout << "Complete. Wrote '" << l_IndexFileName << "'" << endl;
	}
	else
	{
		cerr << "Failed." << endl;
	}

	cout << "Counting..." << endl;
	const char* l_CountFileName = "../3DPhotoBrowser/Binaries/data/photo_index_counts.dat";
	if( WriteIndexCountFile(l_CountFileName) )
	{
		cout << "Complete. Wrote '" << l_CountFileName << "'" << endl;
	}
	else
	{
		cerr << "Failed." << endl;
	}

	cout << "Press enter to continue..." << endl;
	cin.ignore(0x7FFFFFFF, '\n');
	return 0;
}

bool ParseIndexFile(const char* in_File)
{
	// Try to open the index file
	ifstream l_File;
	l_File.open(in_File, ios::binary | ios::in);
	if(l_File.fail())
	{
		cerr << "Failed to open index file reading" << endl;
		return false;
	}

	// Read the number of images
	unsigned l_ImageCount = 0;
	l_File.read((char*)&l_ImageCount, 4);

	// For each image in the index file
	for(unsigned i = 0; i < l_ImageCount; i++)
	{
		// Read the data for each image
		IndexFileImageData l_Data;
		l_File.read((char*)&l_Data, sizeof(IndexFileImageData));
		gImageData.push_back(l_Data);

		// Count the number of images per day as we go along
		vector<unsigned>& l_DayCounts = gDayCounts[l_Data.Year];
		while( l_DayCounts.size() <= (unsigned short)l_Data.DayOfYear )
		{
			l_DayCounts.push_back(0);
		}
		l_DayCounts[l_Data.DayOfYear]++;
	}

	l_File.close();
	return true;
}

bool WriteSortedIndexFile(const char* in_File)
{
	// Sort image data in ascending order
	sort(gImageData.begin(), gImageData.end());

	// Try to open the file
	ofstream l_File;
	l_File.open(in_File, ios::binary | ios::out);
	if(l_File.fail())
	{
		cerr << "Failed to open index file writing" << endl;
		return false;
	}

	// Write the number of images
	unsigned l_ImageCount = gImageData.size();
	l_File.write((char*)&l_ImageCount, 4);

	// For each image in the index file
	for(unsigned i = 0; i < l_ImageCount; i++)
	{
		// Write the data for each image
		IndexFileImageData& l_Data = gImageData[i];
		l_File.write((char*)&l_Data, sizeof(IndexFileImageData));
	}

	l_File.close();
	return true;
}

bool WriteIndexCountFile(const char* in_File)
{
	// Try to open the file
	ofstream l_File;
	l_File.open(in_File, ios::binary | ios::out);
	if(l_File.fail())
	{
		cerr << "Failed to open count file for writing" << endl;
		return false;
	}

	// For each year
	for(map<short, vector<unsigned>>::iterator It = gDayCounts.begin(); It != gDayCounts.end(); It++)
	{
		// For each set of days within each year
		for(unsigned i = 0; i < It->second.size(); i++)
		{
			short l_Year = (short)It->first;
			short l_Day = (short)i;
			unsigned l_Count = It->second[i];

			// Don't bother writing zero entries
			if(l_Count > 0)
			{
				// Write the Year, followed by the day, followed by the count
				l_File.write((char*)&l_Year, sizeof(l_Year));
				l_File.write((char*)&l_Day, sizeof(l_Day));
				l_File.write((char*)&l_Count, sizeof(l_Count));
			}
		}
	}

	l_File.close();
	return true;
}