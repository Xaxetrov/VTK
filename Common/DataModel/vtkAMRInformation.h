/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkAMRInformation.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkAMRInformation - Meta data that describes the structure of an AMR data set
//
// .SECTION Description
// vtkAMRInformation encaspulates the following meta information for an AMR data set
// - a list of vtkAMRBox objects
// - Refinement ratio between AMR levels
// - Grid spacing for each level
// - The file block index for each block
// - parent child information, if requested
//
// .SECTION See Also
// vtkOverlappingAMR, vtkAMRBox

#ifndef __vtkAMRInformation_h
#define __vtkAMRInformation_h

#include "vtkCommonDataModelModule.h" // For export macro
#include "vtkObject.h"
#include "vtkAMRBox.h" //for storing AMR Boxes
#include "vtkSmartPointer.h" //for ivars
#include <vector> //for storing AMR Boxes


typedef std::vector<vtkAMRBox> vtkAMRBoxList;

class vtkUnsignedIntArray;
class vtkIntArray;
class vtkDoubleArray;
class vtkAMRIndexIterator;

class VTKCOMMONDATAMODEL_EXPORT vtkAMRInformation : public vtkObject
{
public:
  static vtkAMRInformation* New();
  vtkTypeMacro(vtkAMRInformation, vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  bool operator==(const vtkAMRInformation& other);

  //Description
  //Initialize the meta information
  // numLevels is the number of levels
  // blocksPerLevel[i] is the number of blocks at level i
  // origin is the global origin
  // gridDescription is the type of vtkUniformGrid, e.g. VTK_XYZ_GRID
  void Initialize(int numLevels, int* numPerBlocks, double origin[3], int gridDescription);

  // Description
  // If this is used, then SetOrigin and SetGridDescription should be called
  // laster
  void Initialize(int numLevels, int* blocksPerLevel);

  // Description:
  // returns the value of vtkUniformGrid::GridDescription() of any block
  vtkGetMacro( GridDescription, int );
  void SetGridDescription(int description);

  // Description:
  // Get the AMR dataset origin
  // The origin is essentially the minimum of all the grids.
  void GetOrigin( double origin[3] );
  double* GetOrigin();
  void SetOrigin( double* origin);

  // Description:
  // Return the number of levels
  unsigned int GetNumberOfLevels() const
  { return static_cast<unsigned int>(this->NumBlocks.size()-1);}

  // Description:
  // Returns the number of datasets at the given levelx
  unsigned int GetNumberOfDataSets(unsigned int level) const;

  // Description:
  // Returns total number of datasets
  unsigned int GetTotalNumberOfBlocks()
  { return this->NumBlocks.back();}

  // Description:
  // Returns the single index from a pair of indices
  unsigned int GetIndex(unsigned int level, unsigned int id) const
  { return this->NumBlocks[level] + id;}

  // Description:
  // Returns the an index pair given a single index
  void ComputeIndexPair(unsigned int index, unsigned int& level, unsigned int& id);

  // Description
  // Returns the bounds of the entire domain
  const double* GetBounds() const {return this->Bounds;}

  // Description
  // Returns the bounding box of a given box
  void GetBounds(unsigned int level, unsigned int id, double* bb);

  // Description
  // Returns the origin of the grid at (level,id)
  bool GetOrigin(unsigned int level, unsigned int id, double* origin);

  //Description
  //Return the spacing at the given fiven
  void GetSpacing(unsigned int level, double spacing[3]);

  // Description:
  // Methods to set and get the AMR box at a given position
  void SetAMRBox(unsigned int level, unsigned int id, const vtkAMRBox& box, double* spacing);
  void SetAMRBox(unsigned int level, unsigned int id, double* min, double* max, int* dimensions);
  void SetAMRBox(unsigned int level, unsigned int id, double* min, int* dimensions, double* h);
  const vtkAMRBox& GetAMRBox(unsigned int level, unsigned int id) const;

  // Description
  // return the amr box coarsened to the previous level
  bool GetCoarsenedAMRBox(unsigned int level, unsigned int id, vtkAMRBox& box) const;

  // Description:
  // Get/Set the SourceIndex of a block. Typically, this is a file-type specific index
  // that can be used by a reader to load a particular file block
  int GetAMRBlockSourceIndex(int index);
  void SetAMRBlockSourceIndex(int index, int sourceId);

  // Description:
  // This method computes the refinement ratio at each level.
  // At each level, l, the refinement ratio r_l is computed by
  // r_l = D_{l} / D_{l+1}, where D_{l+1} and D_{l} are the grid
  // spacings at the next and current level respectively.
  //
  // .SECTION Assumptions
  // 1) Within each level, the refinement ratios are the same for all blocks.
  // 2) The refinement ratio is uniform along each dimension of the block.
  void GenerateRefinementRatio();

  // Description:
  // Returns Wether refinement ratio has been set (either by calling
  // GenerateRefinementRatio() or by calling SetRefinementRatio()
  bool HasRefinementRatio();

  // Description:
  // Set the refinement ratio at a level. This method should be
  // called for all levels, if called at all.
  void SetRefinementRatio(unsigned int level, int ratio);

  // Description:
  // Returns the refinement of a given level.
  int GetRefinementRatio(unsigned int level) const;

  //Description:
  //Return whether parent child information has been generated
  bool HasChildrenInformation();

  // Description:
  // Return a pointer to Parents of a block.  The first entry is the number
  // of parents the block has followed by its parent ids in level-1.
  // If none exits it returns NULL.
  unsigned int *GetParents(unsigned int level, unsigned int index, unsigned int& numParents);

  // Description:
  // Return a pointer to Children of a block.  The first entry is the number
  // of children the block has followed by its childern ids in level+1.
  // If none exits it returns NULL.
  unsigned int *GetChildren(unsigned int level, unsigned int index, unsigned int& numChildren);

  // Description:
  // Prints the parents and children of a requested block (Debug Routine)
  void PrintParentChildInfo(unsigned int level, unsigned int index);

  //Description:
  // Generate the parent/child relationships - needed to be called
  // before GetParents or GetChildren can be used!
  void GenerateParentChildInformation();


  // Description:
  // Checks whether the meta data is internally consistent. Useful for testing
  bool IsValid();

  // Description:
  //Given a point q, find whether q is bounded by the data set at
  //(level,index).  If it is, set cellIdx to the cell index and return
  //true; otherwise return false
  bool FindCell(double q[3],unsigned int level, unsigned int index,int &cellIdx);

  // Description:
  // Returns internal arrays.
  const std::vector<int>& GetNumBlocks() const
  { return this->NumBlocks;}

  std::vector<std::vector<unsigned int> >& GetChildrenAtLevel(unsigned int i)
  { return this->AllChildren[i];}

  void DeepCopy(vtkAMRInformation *other);

 private:
  vtkAMRInformation();
  ~vtkAMRInformation();
  vtkAMRInformation(const vtkAMRInformation&); // Not implemented.
  void operator=(const vtkAMRInformation&); // Not implemented.

  bool HasValidOrigin();
  void UpdateBounds(const int level, const int id);
  void UpdateSpacing(unsigned int level,const double* h);
  void AllocateBoxes(unsigned int n);
  void GenerateBlockLevel();
  void CalculateParentChildRelationShip( unsigned int level,
                                        std::vector<std::vector<unsigned int> >& children,
                                        std::vector<std::vector<unsigned int> >& parents );

  //-------------------------------------------------------------------------
  // Essential information that determines an AMR structure. Must be copied
  //-------------------------------------------------------------------------
  int GridDescription; //example: VTK_XYZ_GRID
  double Origin[3]; //the origin of the whole data set
  vtkAMRBoxList Boxes; // vtkAMRBoxes, one per data set
  std::vector<int> NumBlocks; //NumBlocks[i] stores the total number of blocks from level 0 to level i-1

  vtkSmartPointer<vtkIntArray> SourceIndex; //Typically, this maps to a file block index used by the reader
  vtkSmartPointer<vtkDoubleArray> Spacing; //The grid spacing for all levels
  double Bounds[6]; //the bounds of the entire domain

  //-------------------------------------------------------------------------
  // Auxillary information that be computed
  //-------------------------------------------------------------------------
  vtkSmartPointer<vtkIntArray> Refinement; //refinement ratio between two adjacent levels
  vtkSmartPointer<vtkUnsignedIntArray> BlockLevel; //only necessary if need to call ComputeIndexPair

  //parent child information
  std::vector<std::vector<std::vector<unsigned int> > > AllChildren;
  std::vector<std::vector<std::vector<unsigned int> > > AllParents;
};

#endif