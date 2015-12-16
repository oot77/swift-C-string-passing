//
//  CStringArray.swift
//  collectprintfs
//
//  Created by thgr on 16.12.15.
//  Copyright © 2015 thgr. All rights reserved.
//

import Foundation

//// from https://gist.github.com/neilpa/b430d148d1c5f4ae5ddd

// Is this really the best way to extend the lifetime of C-style strings? The lifetime
// of those passed to the String.withCString closure are only guaranteed valid during
// that call. Tried cheating this by returning the same C string from the closure but it
// gets dealloc'd almost immediately after the closure returns. This isn't terrible when
// dealing with a small number of constant C strings since you can nest closures. But
// this breaks down when it's dynamic, e.g. creating the char** argv array for an exec
// call.
class CString {
    private let _len: Int
    let buffer: UnsafeMutablePointer<Int8>
    
    init(_ string: String) {
        (_len, buffer) = string.withCString {
            let len = Int(strlen($0) + 1)
            let dst = strcpy(UnsafeMutablePointer<Int8>.alloc(len), $0)
            return (len, dst)
        }
    }
    
    deinit {
        buffer.dealloc(_len)
    }
}

// An array of C-style strings (e.g. char**) for easier interop.
class CStringArray {
    // Have to keep the owning CString's alive so that the pointers
    // in our buffer aren't dealloc'd out from under us.
    private let _strings: [CString]
    var numberOfElements:Int = 0
    var pointers: [UnsafeMutablePointer<Int8>]
    
    init(_ strings: [String]) {
        _strings = strings.map { CString($0) }
        pointers = _strings.map { $0.buffer }
        // NULL-terminate our string pointer buffer since things like
        // exec*() and posix_spawn() require this.
        pointers.append(nil)
        self.numberOfElements = strings.count
        //print("\(numberOfElements)")
    }
}

//Usage
//let argv = CStringArray(["ls", "/"])
//posix_spawnp(nil, argv.pointers[0], nil, nil, argv.pointers, nil)

//// END from https://gist.github.com/neilpa/b430d148d1c5f4ae5ddd
