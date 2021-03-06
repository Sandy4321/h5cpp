/*
 * Copyright (c) 2017 vargaconsulting, Toronto,ON Canada
 * Author: Varga, Steven <steven@vargaconsulting.ca>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this  software  and associated documentation files (the "Software"), to deal in
 * the Software  without   restriction, including without limitation the rights to
 * use, copy, modify, merge,  publish,  distribute, sublicense, and/or sell copies
 * of the Software, and to  permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE  SOFTWARE IS  PROVIDED  "AS IS",  WITHOUT  WARRANTY  OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT  SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY  CLAIM,  DAMAGES OR OTHER LIABILITY, WHETHER
 * IN  AN  ACTION  OF  CONTRACT, TORT OR  OTHERWISE, ARISING  FROM,  OUT  OF  OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef  H5CPP_READ_H 
#define H5CPP_READ_H
namespace h5{ namespace impl{ // implementation details namespace

	template<typename T> T* read( hid_t ds, T* ptr, const hsize_t* offset, const hsize_t* count ){

		hid_t file_space = H5Dget_space(ds);
        hsize_t rank = H5Sget_simple_extent_ndims(file_space);
		hid_t mem_type = utils::h5type<T>();
		hid_t mem_space  = H5Screate_simple(rank,count,NULL);

		H5Sselect_hyperslab(file_space, H5S_SELECT_SET, offset, NULL, count, NULL);
		H5Dread(ds, mem_type, mem_space, file_space, H5P_DEFAULT, ptr );
        H5Tclose(mem_type); H5Sclose(file_space); H5Sclose(mem_space);
		return ptr;
	}

	std::vector<std::string> read(hid_t ds, const hsize_t* offset, const hsize_t* count ){

		hid_t file_space = H5Dget_space(ds);
		hsize_t rank = H5Sget_simple_extent_ndims(file_space);
		hid_t mem_space  = H5Screate_simple(rank,count,NULL);
		hid_t mem_type = utils::h5type<std::string>();

		std::vector<std::string> data_set = utils::ctor<std::vector<std::string>>(rank, count );
		// read
		char ** ptr = static_cast<char **>( 
						malloc( data_set.size() * sizeof(char *)));

		H5Sselect_hyperslab(file_space, H5S_SELECT_SET, offset, NULL, count, NULL);
		H5Dread(ds, mem_type, mem_space, file_space, H5P_DEFAULT, ptr );

		for(int i=0; i<data_set.size(); i++)
			if( ptr[i] != NULL )
				data_set[i] = std::string( ptr[i] );
		H5Dvlen_reclaim (mem_type, file_space, H5P_DEFAULT, ptr);
        free(ptr);
		H5Sclose(file_space); H5Sclose(mem_space); H5Tclose(mem_type);
		// end read	
		return data_set;
	}

}}

namespace h5 {
	/** \ingroup io-read 
	 * @brief reads entire HDF5 dataset and returns object defined by T template
	 * @param ds valid HDF5 dataset descriptor
	 * @tparam T stl|arma|eigen templated type    
	 * @return T<sometype> object
	 * \code
	 * example:
	 * 		stl::vector<float> entire_dataset = h5::read<stl::vector<float>>( ds );		
	 * \endcode 
	 */
	template<typename T,typename BaseType = typename utils::base<T>::type> T read( hid_t ds ){

		hid_t file_space = H5Dget_space(ds);
		hsize_t offset[H5CPP_MAX_RANK]={}; // all zeros
		hsize_t count[H5CPP_MAX_RANK];
		hsize_t rank = H5Sget_simple_extent_dims(file_space, count, NULL);
		T data_set = utils::ctor<T>(rank, count );
		BaseType * ptr = utils::data( data_set );
		impl::read(ds, ptr, offset, count);
		return data_set;
	}
	/** \ingroup io-read 
	 * @brief reads entire HDF5 dataset specified by path and returns object defined by T template
	 * @param fd valid HDF5 file descriptor
	 * @param path valid absolute path to HDF5 dataset
	 * @tparam T stl|arma|eigen templated type    
	 * @return T<sometype> object
	 * \code
	 * example:
	 * 		stl::vector<float> entire_dataset = h5::read<stl::vector<float>>( fd,"absolute/path" );		
	 * \endcode 
	 */
	template<typename T> T read( hid_t fd, const std::string& path ){
     	hid_t ds = H5Dopen(fd, path.data(), H5P_DEFAULT);
			const T& data = h5::read<T>(ds);
        H5Dclose(ds);
		return data;
	}
	/** \ingroup io-read 
	 * @brief partial reads HDF5 dataset into a memory region defined by pointer 
	 * @param ds valid HDF5 dataset descriptor
	 * @param ptr pointer to a sufficient size allocated memory where data is loaded
	 * @param offset the coordinates withing HDF5 dataset with rank of the file space, for instance a cube {0,0,0}
	 * @param count amount of data returned, each dimension must be in (1,max_dimension), for instance {1,3,10} returns a matrix
	 * @tparam T [unsigned](char|short|int|long long)|(float|double) type    
	 * @return T<sometype> object 
	 */
	template<typename T> T* read( hid_t ds, T* ptr, std::initializer_list<hsize_t> offset, std::initializer_list<hsize_t> count ){
		return impl::read(ds,ptr,offset.begin(),count.begin() );
	}
	/** \ingroup io-read 
	 * @brief partial reads HDF5 dataset into a memory region defined by pointer 
	 * @param fd valid HDF5 file descriptor
	 * @param path valid absolute path to HDF5 dataset
	 * @param ptr pointer to a sufficient size allocated memory where data is loaded
	 * @param offset the coordinates withing HDF5 dataset with rank of the file space, for instance a cube {0,0,0}
	 * @param count amount of data returned, each dimension must be in (1,max_dimension), for instance {1,3,10} returns a matrix
	 * @tparam T [unsigned](char|short|int|long long)|(float|double) type    
	 * @return T<sometype> object 
	 */
	template<typename T> T* read( hid_t fd, const std::string& path, T* ptr,
			std::initializer_list<hsize_t> offset, std::initializer_list<hsize_t> count ){
     	hid_t ds = H5Dopen(fd, path.data(), H5P_DEFAULT);
			T* data = h5::read<T>(ds,ptr,offset,count);
        H5Dclose(ds);
		return data;
	}
	/** \ingroup io-read 
	 * @brief partial reads HDF5 dataset into T object then returns it  
	 * @param ds valid HDF5 dataset descriptor
	 * @param offset the coordinates withing HDF5 dataset with rank of the file space, for instance a cube {0,0,0}
	 * @param count amount of data returned, each dimension must be in (1,max_dimension), for instance {1,3,10} returns a matrix
	 * @tparam T [unsigned](char|short|int|long long)|(float|double) type    
	 * @return T<some_type> object 
	 */
	template<typename T, typename BaseType = typename utils::base<T>::type > T read( hid_t ds,
			std::initializer_list<hsize_t> offset, std::initializer_list<hsize_t> count ){
		hsize_t rank = count.size();
		T data_set = utils::ctor<T>(rank, count.begin() );
		BaseType * ptr = utils::data( data_set );
		impl::read(ds, ptr, offset.begin(), count.begin());
		return data_set;
	}
	/** \ingroup io-read 
	 * @brief partial reads HDF5 dataset into T object then returns it  
	 * @param fd valid HDF5 file descriptor
	 * @param path valid absolute path to HDF5 dataset
	 * @param offset the coordinates withing HDF5 dataset with rank of the file space, for instance a cube {0,0,0}
	 * @param count amount of data returned, each dimension must be in (1,max_dimension), for instance {1,3,10} returns a matrix
	 * @tparam T [unsigned](char|short|int|long long)|(float|double) type    
	 * @return T<some_type> object 
	 */
	template<typename T> T read( hid_t fd, const std::string& path,
			std::initializer_list<hsize_t> offset, std::initializer_list<hsize_t> count ){ 
		hid_t ds = H5Dopen(fd, path.data(), H5P_DEFAULT);
			T data = h5::read<T>(ds,offset,count);
        H5Dclose(ds);
		return data;

	}

	/** \ingroup io-read 
	 * @brief partial reads HDF5 variable string dataset   
	 * @param ds valid HDF5 dataset descriptor
	 * @return std::vector<std:string> object 
	 */
	template<> std::vector<std::string> read<std::vector<std::string>,std::string>( hid_t ds ){

		hid_t file_space = H5Dget_space(ds);
		hsize_t offset[H5CPP_MAX_RANK]={}; // all zeros
		hsize_t count[H5CPP_MAX_RANK];
		hsize_t rank = H5Sget_simple_extent_dims(file_space, count, NULL);
		return impl::read(ds, offset, count);
	}
	/** \ingroup io-read 
	 * @brief partial reads HDF5 variable string dataset 
	 * @param ds valid HDF5 dataset descriptor
	 * @param offset the coordinates withing HDF5 dataset with rank of the file space, for instance a cube {0,0,0}
	 * @param count amount of data returned, each dimension must be in (1,max_dimension), for instance {1,3,10} returns a matrix
	 * @return std::vector<std:string> object 
	 */
	template<> std::vector<std::string> read<std::vector<std::string>,std::string>(
		   	hid_t ds,std::initializer_list<hsize_t> offset, std::initializer_list<hsize_t> count  ){
		return impl::read(ds, offset.begin(), count.begin() );
	}
}



#endif
