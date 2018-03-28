#ifndef CONTINGENCY_TABLES_HH__
#define CONTINGENCY_TABLES_HH__

#include "ContingencyTable.hh"

#include <utility>
#include <vector>

// A forward reference is sufficient here because we only store
// a *reference* to this class.
class LookupTable;

/*
 @class ContingencyTables
 @brief Manager class for multiple contingency tables

 This class manages a set of contingency tables that belong to a given
 candidate pattern. It permits operating on all tables using a unified
 interface.
*/

class ContingencyTables
{
public:

  /** Creates a new set of contingency tables, which is initially empty */
  ContingencyTables( unsigned n, unsigned n1, LookupTable& lookupTable );

  /**
    Updates all contingency tables by adding a new element with a given
    distance to all tables. The label is required to update the *cells*
    of each contingency table correctly.
  */

  void insert( double distance, bool label );

  // Iterators ---------------------------------------------------------

  using const_iterator = typename std::vector<ContingencyTable>::const_iterator;
  using iterator       = typename std::vector<ContingencyTable>::iterator;

  const_iterator begin() const noexcept { return _tables.begin(); }
  iterator begin()             noexcept { return _tables.begin(); }
  const_iterator end()   const noexcept { return _tables.end();   }
  iterator end()               noexcept { return _tables.end();   }

  // $p-values$ --------------------------------------------------------

  long double min_p()            const noexcept;
  long double min_optimistic_p() const noexcept;

  /*
    Retrieves the table with the lowest $p$-value and returns it for
    further processing. This function should only be called *once*,
    because it has to iterate over all tables.
  */

  std::pair<long double, ContingencyTable> min() const noexcept;

  /**
    Removes all those tables whose optimistic $p$-values is larger than
    the given threshold. Typically, $p$ is Tarone's adjusted threshold,
    so one essentially removes patterns that cannot become testable.

    The function returns the number of tables that have been pruned.
  */

  std::size_t prune( long double p );

  // Returns the number of tables that are left after performing
  // a pruning process. Ideally, this should only be *one* table
  // but edge cases in data might lead to different results.
  std::size_t size() const noexcept
  {
    return _tables.size();
  }

  /** @returns true if the set of contingency tables is empty */
  bool empty() const noexcept
  {
    return _tables.empty();
  }

private:
  unsigned _n;
  unsigned _n1;

  // Lookup table for quickly calculating minimum attainable $p$-values
  // of contingency tables.
  LookupTable& _lookupTable;

  // All contingency tables stored by the manager. This container is
  // reduced by the pruning operation.
  std::vector<ContingencyTable> _tables;

  // Short-hand for a pair of a distance value and its corresponding
  // label. This is only defined for convenience purposes.
  using DistanceLabelPair = std::pair<double, bool>;

  // All distance--label pairs that have already been encountered. It is
  // important to keep them in order to update new tables. No sorting of
  // any kind will be performed.
  std::vector<DistanceLabelPair> _distanceLabelPairs;
};

#endif
