#ifndef BL_DISTRIBUTIONMAPPING_H
#define BL_DISTRIBUTIONMAPPING_H

#include <map>
#include <limits>

#include <Pointers.H>
#include <BoxLib.H>
#include <Array.H>
#include <Box.H>
#include <REAL.H>

class BoxArray;

//
// Calculates the distribution of FABs to MPI processes.
//
//  This class calculates the distribution of FABs to MPI processes in a
//  FabArray in a multi-processor environment.  By distribution is meant what
//  MPI process in the multi-processor environment owns what FAB.  Only the BoxArray
//  on which the FabArray is built is used in determining the distribution.
//  The three types of distributions supported are round-robin, knapsack, and SFC.
//  In the round-robin distribution FAB i is owned by CPU i%N where N is total
//  number of CPUs.  In the knapsack distribution the FABs are partitioned
//  across CPUs such that the total volume of the Boxes in the underlying
//  BoxArray are as equal across CPUs as is possible.  The SFC distribution is
//  based on a space filling curve.
//

class DistributionMapping
{
  public:
    //
    // The distribution strategies
    //
    enum Strategy { ROUNDROBIN, KNAPSACK, SFC, PFC, RRSFC };
    //
    // The default constructor.
    //
    DistributionMapping ();
    //
    // Create an object with the specified mapping.
    // VERY IMPORTANT:  The size of pmap must be (yourboxarray.size() + 1).
    //                  The value in pmap[yourboxarray.size()] is a sentinel used
    //                  in boxlib and its value must be set to
    //                  ParallelDescriptor::MyProc() by the caller.
    //
    DistributionMapping (const Array<int>& pmap, bool put_in_cache = false);
    //
    // Build mapping out of BoxArray over nprocs processors.
    //
    DistributionMapping (const BoxArray& boxes, int nprocs);
    //
    // This is a very specialized distribution map.
    // Do NOT use it unless you really understand what it does.
    //
    DistributionMapping (const DistributionMapping& d1,
                         const DistributionMapping& d2);
    //
    // The destructor.
    //
    ~DistributionMapping ();
    //
    // Copy constructor.
    //
    DistributionMapping (const DistributionMapping& rhs);
    //
    // Assignment operator.
    //
    DistributionMapping& operator= (const DistributionMapping& rhs);
    //
    // Build mapping out of BoxArray over nprocs processors.
    // You need to call this if you built your DistributionMapping
    // with the default constructor.
    //
    void define (const BoxArray& boxes, int nprocs);
    //
    // Returns a constant reference to the mapping of boxes in the
    // underlying BoxArray to the CPU that holds the FAB on that Box.
    // ProcessorMap()[i] is an integer in the interval [0, NCPU) where
    // NCPU is the number of CPUs being used.
    //
    const Array<int>& ProcessorMap () const;
    //
    // Length of the underlying processor map.
    //
    int size () const { return m_ref->m_pmap.size(); }
    //
    // Equivalent to ProcessorMap()[index].
    //
    int operator[] (int index) const { return m_ref->m_pmap[index]; }
    //
    // Set/get the distribution strategy.
    //
    static void strategy (Strategy how);

    static Strategy strategy ();
    //
    // Set/get the space filling curve threshold.
    //
    static void SFC_Threshold (int n);

    static int SFC_Threshold ();
    //
    // Flush the cache of processor maps.  The processor map cache
    // is only flushed manually.  Only call this after a regridding
    // before new MultiFabs are alloc()d.  This removes unreferenced
    // DistributionMaps from the Cache.
    //
    static void FlushCache ();
    //
    // The size of the cache.
    //
    static int CacheSize ();
    //
    // Output some simple cache statistics.
    //
    static void CacheStats (std::ostream& os);
    //
    // Put in cache
    //
    void PutInCache();
    //
    // Are the distributions equal?
    //
    bool operator== (const DistributionMapping& rhs) const;
    //
    // Are the distributions different?
    //
    bool operator!= (const DistributionMapping& rhs) const;

    void SFCProcessorMap(const BoxArray& boxes, const std::vector<long>& wgts,
                         int nprocs);
    void PFCProcessorMap(const BoxArray& boxes, const std::vector<long>& wgts,
                         int nprocs);
    void KnapSackProcessorMap(const std::vector<long>& wgts, int nprocs,
                              Real* efficiency = 0,
			      bool do_full_knapsack = true,
			      int nmax = std::numeric_limits<int>::max());
    void RoundRobinProcessorMap(int nboxes, int nprocs);
    void RRSFCProcessorMap(const BoxArray&boxes, int nprocs);
    //
    // Initializes distribution strategy from ParmParse.
    //
    // ParmParse options are:
    //
    //   DistributionMapping.strategy = ROUNDROBIN
    //   DistributionMapping.strategy = KNAPSACK
    //   DistributionMapping.strategy = SFC
    //   DistributionMapping.strategy = PFC
    //   DistributionMapping.strategy = RRFC
    //
    static void Initialize ();

    static void Finalize ();

    static bool SameRefs (const DistributionMapping& lhs,
                          const DistributionMapping& rhs)
		  { return lhs.m_ref == rhs.m_ref; }

    static void PrintDiagnostics(const std::string &filename);
    //
    // Initialize the topological proximity map
    //
    static void InitProximityMap();
    static int NHops(const Box &tbox, const IntVect &ivfrom, const IntVect &ivto);
    static int ProcNumberFromRank(const int rank);
    static std::vector<int> RanksFromProcNumber(const int procnum);
    static IntVect TopIVFromProcNumber(const int procnum);
    static std::vector<int> ProcNumbersFromTopIV(const IntVect &iv);
    static IntVect TopIVFromRank(const int rank);
    static std::vector<int> RanksFromTopIV(const IntVect &iv);
    static std::string GetProcName();
    static int GetProcNumber();
    static int ProximityMap(const int rank)   { return proximityMap[rank];   }
    static int ProximityOrder(const int rank) { return proximityOrder[rank]; }

    static void ReadCheckPointHeader(const std::string &filename,
				     Array<IntVect>  &refRatio,
                                     Array<BoxArray> &allBoxes);
    static void PFCMultiLevelMap(const Array<IntVect>  &refRatio,
                                 const Array<BoxArray> &allBoxes);


private:
    //
    // Ways to create the processor map.
    //
    void RoundRobinProcessorMap (const BoxArray& boxes, int nprocs);
    void KnapSackProcessorMap   (const BoxArray& boxes, int nprocs);
    void SFCProcessorMap        (const BoxArray& boxes, int nprocs);
    void PFCProcessorMap        (const BoxArray& boxes, int nprocs);
    void RRFCProcessorMap       (const BoxArray& boxes, int nprocs);

    typedef std::pair<long,int> LIpair;

    struct LIpairLT
    {
        bool operator () (const LIpair& lhs,
                          const LIpair& rhs) const
            {
                return lhs.first < rhs.first;
            }
    };

    struct LIpairGT
    {
        bool operator () (const LIpair& lhs,
                          const LIpair& rhs) const
            {
                return lhs.first > rhs.first;
            }
    };

    static void Sort (std::vector<LIpair>& vec, bool reverse);

    void RoundRobinDoIt (int                  nboxes,
                         int                  nprocs,
                         std::vector<LIpair>* LIpairV = 0);

    void KnapSackDoIt (const std::vector<long>& wgts,
                       int                      nprocs,
                       Real&                    efficiency,
                       bool                     do_full_knapsack,
		       int                      nmax=std::numeric_limits<int>::max());

    void SFCProcessorMapDoIt (const BoxArray&          boxes,
                              const std::vector<long>& wgts,
                              int                      nprocs);

    void PFCProcessorMapDoIt (const BoxArray&          boxes,
                              const std::vector<long>& wgts,
                              int                      nprocs);

    void RRSFCDoIt           (const BoxArray&          boxes,
                              int                      nprocs);

    //
    // Current # of bytes of FAB data.
    //
    static void CurrentBytesUsed (int nprocs, Array<long>& result);
    static void CurrentCellsUsed (int nprocs, Array<long>& result);
    //
    // Least used ordering of CPUs (by # of bytes of FAB data).
    //
    static void LeastUsedCPUs (int nprocs, Array<int>& result);
    //
    // Look for a cached processor map.
    //
    bool GetMap (const BoxArray& boxes);
    //
    // A useful typedef.
    //
    typedef void (DistributionMapping::*PVMF)(const BoxArray &, int);
    //
    // Everyone uses the same Strategy -- defaults to SFC.
    //
    static Strategy m_Strategy;
    //
    // Pointer to one of the CreateProcessorMap() functions.
    // Corresponds to the one specified by m_Strategy.
    //
    static PVMF m_BuildMap;

    class Ref
    {
        friend class DistributionMapping;
        //
        // Constructors to match those in DistributionMapping ....
        //
        Ref ();

        Ref (int len);

        Ref (const Array<int>& pmap);

        Ref (const Ref& rhs);
        //
        // Local data -- our processor map.
        //
        // The length is always equal to boxes.length()+1 where boxes is
        // the BoxArray on which the distribution is based.  It is also true
        // that m_pmap[boxes.length()] == ParallelDescriptor::MyProc().
        // This latter acts as a sentinel in some FabArray loops.
        //
        Array<int> m_pmap;
        //Array<long> boxPoints;  // i == rank.  so we can subtract from total
    };
    //
    // The data -- a reference-counted pointer to a Ref.
    //
    LnClassPtr<Ref> m_ref;
    //
    // Our cache of processor maps.
    //
    static std::map< int,LnClassPtr<Ref> > m_Cache;
    //
    // Topological proximity map
    //
    static long totalCells;
    static Real bytesPerCell;
    static Array<int> proximityMap;    // i == rank, pMap[i]   == proximity mapped rank
    static Array<int> proximityOrder;  // i == rank, pOrder[i] == proximity mapped order
    static Array<long> totalBoxPoints;  // i == rank
    //const Array<long> &BoxPoints() { return boxPoints; }
};

//
// Our output operator.
//
std::ostream& operator<< (std::ostream& os, const DistributionMapping& pmap);

#endif /*BL_DISTRIBUTIONMAPPING_H*/
