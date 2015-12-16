//
//  ViewController.swift
//  collectprintfs
//
//  Created by thgr on 15.12.15.
//  Copyright © 2015 thgr. All rights reserved.
//

import UIKit

class tViewController: UIViewController {

    @IBOutlet weak var displayReturnStringTextView: UITextView!
    
    var myArgs = CStringArray(["runIT", "hi", "there", "!"])

    @IBOutlet weak var argvTextInput: UITextField!
    @IBAction func argvTextInputReturn(sender: UITextField) {
    }

    @IBAction func tRun(sender: UIButton) {
        //displayReturnStringTextView.text = "Hi there"
        if let argv = argvTextInput.text {
            myArgs=CStringArray(argv.characters.split{$0 == " "}.map(String.init))
        }
        
        let returnedString = tmain(myArgs.numberOfElements, &myArgs.pointers[0])

        if let tReturns = String.fromCString(returnedString) {
            displayReturnStringTextView.text = tReturns
        }

        tFreeMemory()
    }

}

