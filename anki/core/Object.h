#ifndef ANKI_CORE_OBJECT_H
#define ANKI_CORE_OBJECT_H

#include <vector>


namespace anki {


/// A class for automatic garbage collection. Cause we -the programmers- get
/// bored when it comes to deallocation. Dont even think to put as a parent an
/// object that has not created dynamically
class Object
{
	public:
		typedef std::vector<Object*> Container;

		/// Calls addChild if parent is not NULL
		/// @exception Exception
		Object(Object* parent);

		/// Delete childs from the last entered to the first and update parent
		virtual ~Object();

	protected:
		/// @name Accessors
		/// @{
		const Object* getObjParent() const {return objParent;}
		Object* getObjParent() {return objParent;}
		const Container& getObjChildren() const {return objChilds;}
		Container& getObjChildren() {return objChilds;}
		/// @}

		void addChild(Object* child);
		void removeChild(Object* child);

	private:
		Object* objParent; ///< May be nullptr
		Container objChilds;
};


} // end namespace


#endif